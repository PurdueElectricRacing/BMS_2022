/*
 * afe.c
 *
 * Created: 9/19/20
 */

#include "afe.h"

extern void change_baud(uint32_t freq, uint32_t os);

// @funcname: crc_16_ibm()
//
// @brief: Calculates the CRC for the AFE. This function is taken from page 55 of
//         the AFE datasheet
//
// @param: data: Data going to AFE
// @param: len: Length of the data
//
// @return: 16 bit CRC value
static uint16_t crc_16_ibm(uint8_t* data, uint16_t len)
{
    // Locals
    uint16_t crc = 0;                                                               // Calculated CRC value
    uint16_t i;                                                                     // Generic counter variable

    while (len--)
    {
        crc ^= *data++;                                                             // I hate this line. Not clear what's happening. Need to fix it
        for (i = 0; i < 8; ++i)                                                     // Loop through CRC
        {
            crc = (crc >> 1) ^ ((crc & 1) ? 0xA001 : 0);                            // Update CRC
        }
    }

    return crc;                                                                     // Return crc
}

// @funcname: tx()
//
// @brief: Adds UART frame to queue. Comes from update_lcd (Matt Flanagan)
//
// @param: data: Data going to AFE
// @param: len: Length of the data
static void tx(uint8_t* data, uint8_t len)
{
    // Locals
    uart_tx_t tx;                                                                   // UART TX struct

    tx.tx_size = len;                                                               // Copy over length
    memcpy(&tx.tx_buffer[0], data, len);                                            // Copy data to struct

    // TODO: Add a timeout for sending values and change return type to a success enumeration

    HAL_UART_Transmit_IT(&huart1, tx.tx_buffer, tx.tx_size);
}

// @funcname: calcBalance
//
// @brief: Calculates which cells should balance
void calcBalance()
{
    // Locals
    uint8_t i;                                                                  // Loop control variable

    for (i = 0; i < bms.module_params.cells_series; i++)                        // Loop through each cell
    {
        if (bms.cells.chan_volts_conv[i] > CELL_UV_THRESH)                      // CASE 1: Cell has an overvolt condition
        {
            bms.cells.balance_flags |= 1U << i;                                 // Flag cell as requiring balancing
        }
        else if (bms.cells.est_SOC[i] > 100)                                    // CASE 2: Cell has a high SOC event
        {
            bms.cells.balance_flags |= 1U << i;                                 // Flag cell as requiring balancing
        }
        // TODO: Implement SOC model
//        else if ((bms.cells.est_cap[i] > bms.cells.est_cap_max) && TRACK_CAP)   // CASE 3: Cell has an estimated capacity higher than what SOH says is possible
//        {
//            bms.cells.balance_flags |= 1U << i;                                 // Flag cell as requiring balancing
//        }
//        else if (bms.cells.est_SOC[i] > bms.cells.avg_SOC + SOC_THRESH)         // CASE 4: Cell has a much higher SOC than other cells
//        {
//            bms.cells.balance_flags |= 1U << i;                                 // Flag cell as requiring balancing
//        }
        else                                                                    // CASE 5: Cell is within limits
        {
            bms.cells.balance_flags &= ~(1U << i);                              // Clear balance flag
        }
    }

    bms.cells.balance_flags &= ~bms.cells.balance_mask;                         // Mask cells with open wire issues (or other issues)
    bms.cells.balance_flags |= bms.override.balance_force;                      // Set flags for cells that user manually set
}

static void wakeup()
{
    // Locals
    uint8_t   cmd[4];                                                               // Command bytes (no device address/group id byte)
    uint16_t  crc;                                                                  // Calculated CRC for data

    // Send wakeup tone
    cmd[0] = RESET | (CMD_FRAME << 7) | (WRITE_BRD_WOR << 4) | (BIT_8 << 3) | BYTE_1;
    cmd[1] = DEV_CTRL;
    cmd[2] = 0b00101000;
    crc = crc_16_ibm(cmd, 4);
    cmd[3] = crc >> 8;
    cmd[4] = (uint8_t) crc;
    tx(cmd, 5);
}

// @funcname: afe_init()
//
// @brief: Initializes the AFE to run at 1 Mb, and precomputes required values
//
// @exec_period: 1000Hz or every 1 ms
void initAfe()
{
    // Locals
    uint8_t   cmd[8];                                                               // Command bytes (no device address/group id byte)
    uint16_t  crc;                                                                  // Calculated CRC for data
    float     thresh;                                                               // Calculated OV/UV threshold on 25 mV scale

    if (bms.afe.comm_rate != BAUD_1_M)                                              // Check if this is the first time we've booted up
    {
        change_baud(250000, FCLK);                                                  // Call assembly subroutine to change baud to 250 kb
        wakeup();                                                                   // Send wakeup tone to each device in stack
        HAL_Delay(5);                                                               // Wait for wakup tone to complete

        // Command to change baud to 1 M
        cmd[0] = RESET | (CMD_FRAME << 7) | (WRITE_BRD_WOR << 4) | (BIT_8 << 3) | BYTE_2;
        cmd[1] = COMCONFIG;
        cmd[2] = 0b11111000;
        cmd[3] = 0b00110000;
        crc = crc_16_ibm(cmd, 4);
        cmd[4] = crc >> 8;
        cmd[5] = (uint8_t) crc;
        tx(cmd, 6);
        change_baud(1000000, FCLK);                                                 // Call assembly subroutine to change baud to 1 Mb
    } else {
        wakeup();                                                                   // Just send wakeup
    }

    // Set UART transmitter holdoff (waits 5 bit periods)
    cmd[0] = RESET | (CMD_FRAME << 7) | (WRITE_BRD_WOR << 4) | (BIT_8 << 3) | BYTE_1;
    cmd[1] = TXHOLDOFF;
    cmd[2] = 5;
    crc = crc_16_ibm(cmd, 3);
    cmd[3] = crc >> 8;
    cmd[4] = (uint8_t) crc;
    tx(cmd, 5);

    // Set channel count
    cmd[1] = NCHAN;
    cmd[2] = bms.module_params.cells_series;                                        // NOTE: This line will need to be changed if we use two AFEs
    crc = crc_16_ibm(cmd, 3);
    cmd[3] = crc >> 8;
    cmd[4] = (uint8_t) crc;
    tx(cmd, 5);

    // Set device configuration (regulator on, addr auto, OV/UV on, comp hyst on, fault latch off)
    cmd[1] = DEVCONFIG;
    cmd[2] = 0b00010011;
    crc = crc_16_ibm(cmd, 3);
    cmd[3] = crc >> 8;
    cmd[4] = (uint8_t) crc;
    tx(cmd, 5);

    // Set power configuration as per datasheet
    cmd[1] = PWRCONFIG;
    cmd[2] = 0b10000000;
    crc = crc_16_ibm(cmd, 3);
    cmd[3] = crc >> 8;
    cmd[4] = (uint8_t) crc;
    tx(cmd, 5);

    // Set balance configuration (will not allow balance with a fault, with auto stop after 1 minute)
    cmd[1] = CBCONFIG;
    cmd[2] = 0b00100000;
    crc = crc_16_ibm(cmd, 3);
    cmd[3] = crc >> 8;
    cmd[4] = (uint8_t) crc;
    tx(cmd, 5);

    // Set initial test configuration
    cmd[0] = RESET | (CMD_FRAME << 7) | (WRITE_BRD_WOR << 4) | (BIT_8 << 3) | BYTE_2;
    cmd[1] = TSTCONFIG;
    cmd[2] = 0b00000101;
    cmd[3] = 0b00000000;
    crc = crc_16_ibm(cmd, 4);
    cmd[4] = crc >> 8;
    cmd[5] = (uint8_t) crc;
    tx(cmd, 6);

    // Set comms timeout (1 second, shutdown, 5 minutes, fault)
    cmd[0] = RESET | (CMD_FRAME << 7) | (WRITE_BRD_WOR << 4) | (BIT_8 << 3) | BYTE_1;
    cmd[1] = CTO;
    cmd[2] = 0b01001001;
    crc = crc_16_ibm(cmd, 3);
    cmd[3] = crc >> 8;
    cmd[4] = (uint8_t) crc;
    tx(cmd, 5);

    // Set auto monitor period (200 ms)
    cmd[1] = AM_PER;
    cmd[2] = 0b00000111;
    crc = crc_16_ibm(cmd, 3);
    cmd[3] = crc >> 8;
    cmd[4] = (uint8_t) crc;
    tx(cmd, 5);

    // TODO: THIS NEEDS TO BE SENT TO EACH AFE IF WE GO TO TWO
    // Set auto monitor channels (15 cells)
    cmd[0] = RESET | (CMD_FRAME << 7) | (WRITE_BRD_WOR << 4) | (BIT_8 << 3) | BYTE_4;
    cmd[1] = AM_PER;
    cmd[2] = 0b01111111;
    cmd[3] = 0b11111111;
    cmd[4] = 0b00000000;
    cmd[5] = 0b00000000;
    crc = crc_16_ibm(cmd, 6);
    cmd[6] = crc >> 8;
    cmd[7] = (uint8_t) crc;
    tx(cmd, 8);

    // Set auto monitor oversampling
    cmd[0] = RESET | (CMD_FRAME << 7) | (WRITE_BRD_WOR << 4) | (BIT_8 << 3) | BYTE_1;
    cmd[1] = AM_OSMPL;
    cmd[2] = 0b01111010;
    crc = crc_16_ibm(cmd, 3);
    cmd[3] = crc >> 8;
    cmd[4] = (uint8_t) crc;
    tx(cmd, 5);

    // Set initial sampling delay
    cmd[0] = RESET | (CMD_FRAME << 7) | (WRITE_BRD_WOR << 4) | (BIT_8 << 3) | BYTE_1;
    cmd[1] = SMPL_DLY1;
    cmd[2] = 0b00000000;
    crc = crc_16_ibm(cmd, 3);
    cmd[3] = crc >> 8;
    cmd[4] = (uint8_t) crc;
    tx(cmd, 5);

    // Set comparator UV threshold
    cmd[0] = RESET | (CMD_FRAME << 7) | (WRITE_BRD_WOR << 4) | (BIT_8 << 3) | BYTE_1;
    cmd[1] = COMP_UV;
    thresh = CELL_UV_THRESH * THRESH_CONV;
    cmd[2] = ((uint8_t) thresh) << 1;
    crc = crc_16_ibm(cmd, 3);
    cmd[3] = crc >> 8;
    cmd[4] = (uint8_t) crc;
    tx(cmd, 5);

    // Set comparator OV threshold
    cmd[0] = RESET | (CMD_FRAME << 7) | (WRITE_BRD_WOR << 4) | (BIT_8 << 3) | BYTE_1;
    cmd[1] = COMP_OV;
    thresh = CELL_OV_THRESH * THRESH_CONV;
    cmd[2] = ((uint8_t) thresh) << 1;
    crc = crc_16_ibm(cmd, 3);
    cmd[3] = crc >> 8;
    cmd[4] = (uint8_t) crc;
    tx(cmd, 5);

    // Set cell UV threshold
    cmd[0] = RESET | (CMD_FRAME << 7) | (WRITE_BRD_WOR << 4) | (BIT_8 << 3) | BYTE_2;
    cmd[1] = CELL_UV;
    thresh = CELL_OV_THRESH * THRESH_CONV;
    cmd[2] = ((uint8_t) thresh) << 1;
    // TODO: Set these to what battery wants
    cmd[3] = 0x00;
    crc = crc_16_ibm(cmd, 4);
    cmd[4] = crc >> 8;
    cmd[5] = (uint8_t) crc;
    tx(cmd, 6);

    // Set compacellrator OV threshold
    cmd[0] = RESET | (CMD_FRAME << 7) | (WRITE_BRD_WOR << 4) | (BIT_8 << 3) | BYTE_2;
    cmd[1] = CELL_OV;
    thresh = CELL_OV_THRESH * THRESH_CONV;
    cmd[2] = ((uint8_t) thresh) << 1;
    // TODO: Set these to what battery wants
    cmd[3] = 0x00;
    crc = crc_16_ibm(cmd, 4);
    cmd[4] = crc >> 8;
    cmd[5] = (uint8_t) crc;
    tx(cmd, 6);

    scale_factor = (2 * V_REF) / 65535;                                             // Precompute scale factor so we don't run this calculation every time
}

// @funcname: afe_conv()
//
// @brief: Converts raw ADC values to engineering units
void afeConv()
{
    // Locals
    uint8_t i;                                                                      // Generic counter variable

    for (i = 0; i < bms.module_params.cells_series; ++i)                            // We are assuming parameters are loaded by the time we get here first
    {
        bms.cells.chan_volts_conv[i] = scale_factor * bms.cells.chan_volts_raw[i];  // Convert raw ADC value to voltage measurement
    }
    bms.cells.mod_volts_conv = scale_factor * bms.cells.mod_volts_raw * 25;         // Convert raw ADC value to voltage measurement
    bms.afe.vm_conv          = scale_factor * bms.afe.vm_raw * -2;                  // Convert raw ADC value to voltage measurement
    bms.afe.v5vao_conv       = scale_factor * bms.afe.v5vao_raw;                    // Convert raw ADC value to voltage measurement
    bms.afe.vdig_conv        = scale_factor * bms.afe.vdig_raw;                     // Convert raw ADC value to voltage measurement
    bms.afe.vdd18_conv       = scale_factor * bms.afe.vdd18_raw;                    // Convert raw ADC value to voltage measurement
}

void afeProcess()
{
    // Locals
    uint8_t   cmd[8];                                                               // Command bytes (no device address/group id byte)
    uint16_t  crc;                                                                  // Calculated CRC for data
}
