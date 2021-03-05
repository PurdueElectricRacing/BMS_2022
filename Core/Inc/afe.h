#ifndef _AFE_H_
#define _AFE_H_

// Includes
#include "bms.h"
#include "string.h"

// Generic Defines
#define V_REF               2.5     // Internal VREF value
#define RESET               0x0     // Command reset value
#define TIMEOUT             5       // Tx/rx timeout
#define WAIT_QUEUE_FULL     30      // Queue full timeout
#define THRESH_CONV         0.025f  // OV/UV conversion factor

// Frame Types
#define RESP_FRAME          0x0     // Response frame
#define CMD_FRAME           0x1     // Command frame

// Request Types
#define WRITE_SINGLE_R      0x0     // Single device write with response
#define WRITE_SINGE_WOR     0x1     // Single device write without response
#define WRITE_GROUP_R       0x2     // Group write with response
#define WRITE_GROUP_WOR     0x3     // Group write without response
#define WRITE_BRD_R         0x6     // Broadcast write with response
#define WRITE_BRD_WOR       0x7     // Broadcast write without response

// Address Sizes
#define BIT_8               0x0     // 8-bit register address
#define BIT_16              0x1     // 16-bit register address

// Data Sizes
#define BYTE_0              0x0     // 0 bytes
#define BYTE_1              0x1     // 1 byte
#define BYTE_2              0x2     // 2 bytes
#define BYTE_3              0x3     // 3 bytes
#define BYTE_4              0x4     // 4 bytes
#define BYTE_5              0x5     // 5 bytes
#define BYTE_6              0x6     // 6 bytes
#define BYTE_8              0x7     // 8 bytes

// Registers (Page 67)
#define SREV                0x00    // Silicon revision
#define CMD                 0x02    // Command
#define CHANNELS            0x03    // Command channel select
#define OVERSMPL            0x07    // Command averaging (oversampling)
#define ADDR                0x0A    // Device address
#define GROUP_ID            0x0B    // (Device) Group Identifier
#define DEV_CTRL            0x0C    // Device control
#define NCHAN               0x0D    // Number of channels for conversion
#define DEVCONFIG           0x0E    // Device configuration
#define PWRCONFIG           0x0F    // Power configuration
#define COMCONFIG           0x10    // Communications configuration
#define TXHOLDOFF           0x12    // UART Transmitter holdoff
#define CBCONFIG            0x13    // Cell balancing (equalization) configuration
#define CBENBL              0x14    // Cell balancing enables
#define TSTCONFIG           0x1E    // Built-In Self-Test (BIST) configuration
#define TESTCTRL            0x20    // BIST control
#define TEST_ADC            0x22    // ADC BIST control
#define TESTAUXPU           0x25    // Test control -- AUX pull-up resistors
#define CTO                 0x28    // Communications time-out
#define CTO_CNT             0x29    // Communications time-out counter
#define AM_PER              0x32    // Auto-monitor period
#define AM_CHAN             0x33    // Auto-monitor channel select
#define AM_OSMPL            0x37    // Auto-monitor averaging
#define SMPL_DLY1           0x3D    // Initial sampling delay
#define CELL_SPER           0x3E    // Cell and die temperature measurement period
#define AUX_SPER            0x3F    // AUX channels sampling period
#define TEST_SPER           0x43    // ADC test sampling period
#define SHDN_STS            0x50    // Shutdown recovery status
#define STATUS              0x51    // Device status
#define FAULT_SUM           0x52    // Fault summary
#define FAULT_UV            0x54    // Undervoltage faults
#define FAULT_OV            0x56    // Overvoltage faults
#define FAULT_AUX           0x58    // AUX threshold exceeded faults
#define FAULT_2UV           0x5A    // Somparator UV faults

// Registers (Page 68)
#define FAULT_2OV           0x5C    // Comparator OV faults
#define FAULT_COM           0x5E    // Communication faults
#define FAULT_SYS           0x60    // System fault
#define FAULT_DEV           0x61    // Device fault
#define FAULT_GPI           0x63    // General purpose input (GPIO) fault
#define MASK_COMM           0x68    // Communications FAULT mask register
#define MASK_SYS            0x6A    // System FAULT mask register
#define MASK_DEV            0x6B    // Chip fault mask register
#define FO_CTRL             0x6E    // FAULT output control
#define GPIO_DIR            0x78    // GPIO direction control
#define GPIO_OUT            0x79    // GPIO output control
#define GPIO_PU             0x7A    // GPIO pull-up resistor control
#define GPIO_PD             0x7B    // GPIO pull-down resistor control
#define GPIO_IN             0x7C    // GPIO input value
#define GP_FILT_IN          0x7D    // GPIO input 0/1 FAULT assertion state
#define MAGIC1              0x82    // "Magic" value enables EEPROM write
#define COMP_UV             0x8C    // Comparator undervoltage threshold
#define COMP_OV             0x8D    // Comparator overvoltage threshold
#define CELL_UV             0x8E    // Cell undervoltage threshold
#define CELL_OV             0x90    // Cell overvoltage threshold
#define AUX0_UV             0x92    // AUX0 undervoltage threshold
#define AUX0_OV             0x94    // AUX0 overvoltage threshold
#define AUX1_UV             0x96    // AUX1 undervoltage threshold
#define AUX1_OV             0x98    // AUX1 overvoltage threshold
#define AUX2_UV             0x9A    // AUX2 undervoltage threshold
#define AUX2_OV             0x9C    // AUX2 overvoltage threshold
#define AUX3_UV             0x9E    // AUX3 undervoltage threshold
#define AUX3_OV             0xA0    // AUX3 overvoltage threshold
#define AUX4_UV             0xA2    // AUX4 undervoltage threshold
#define AUX4_OV             0xA4    // AUX4 overvoltage threshold
#define AUX5_UV             0xA6    // AUX5 undervoltage threshold
#define AUX5_OV             0xA8    // AUX5 overvoltage threshold
#define AUX6_UV             0xAA    // AUX6 undervoltage threshold
#define AUX6_OV             0xAC    // AUX6 overvoltage threshold
#define AUX7_UV             0xAE    // AUX7 undervoltage threshold
#define AUX7_OV             0xB0    // AUX7 overvoltage threshold
#define LOT_NUM             0xBE    // Device Lot Number
#define SER_NUM             0xC6    // Device Serial Number
#define SCRATCH             0xC8    // User-defined data
#define VSOFFSET            0xD2    // ADC voltage offset correction

// Registers (Page 69 [Nice])
#define VSGAIN              0xD3    // ADC voltage gain correction
#define AX0OFFSET           0xD4    // AUX0 ADC offset correction
#define AX1OFFSET           0xD6    // AUX1 ADC offset correction
#define AX2OFFSET           0xD7    // AUX2 ADC offset correction
#define AX3OFFSET           0xDA    // AUX3 ADC offset correction
#define AX4OFFSET           0xDC    // AUX4 ADC offset correction
#define AX5OFFSET           0xDE    // AUX5 ADC offset correction
#define AX6OFFSET           0xE0    // AUX6 ADC offset correction
#define AX7OFFSET           0xE2    // AUX7 ADC offset correction
#define TSTR_ECC            0xE6    // ECC Test Results
#define CSUM                0xF0    // Saved checksum value
#define CSUM_RSLT           0xF4    // Checksum Readout
#define TEST_CSUM           0xF8    // Checksum Test Result
#define EE_BURN             0xFA    // EEPROM Burn Count; up-counter
#define MAGIC2              0xFC    // "Magic" value enables EEPROM write

// Enumerations
typedef enum {
    INIT,                           // Initialization state
    SLEEP,                          // Sleep state
    AFE_STATE_COUNT                 // Must be last
} afe_state_t;

// Precomputed Values
float scale_factor;                 // (2 * V_REF) / 65535

// Prototypes
void afeInit();                     // Initializes the AFE and ups baud on first boot
void afeProcess();                  // Communicates with AFE and pulls ADC values
void calcBalance();                 // Determines which cells require balancing

#endif