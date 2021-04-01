#include "can.h"

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef* hcan)
{
    // Locals
    CanRxMsgTypeDef rx;                                             // Rx struct
    CAN_RxHeaderTypeDef header;                                     // Header struct

    HAL_CAN_GetRxMessage(hcan, 0, &header, rx.Data);                // Get the message we just rx'd
    rx.DLC = header.DLC;                                            // Copy length
    rx.StdId = header.StdId;                                        // Copy ID

    qSendToBack(&bms.q_rx_can, &rx);                                // Add to queue (qSendToBack is interrupt safe)
}

void txCan()
{
    // Locals
    CanTxMsgTypeDef     tx;                                         // Tx frame to send
    CAN_TxHeaderTypeDef header;                                     // Header frame for send
    uint32_t            mailbox;                                    // Mailbox for send

    if (qReceive(&bms.q_tx_can, &tx) == SUCCESS_G)                  // Check queue for items and take if there is one
    {
        header.DLC = tx.DLC;                                        // Copy frame length
        header.IDE = tx.IDE;                                        // Copy frame ID length
        header.RTR = tx.RTR;                                        // Copy frame type
        header.StdId = tx.StdId;                                    // Copy frame ID
        header.TransmitGlobalTime = DISABLE;                        // Don't send timestamp counter value

        while (!HAL_CAN_GetTxMailboxesFreeLevel(bms.can));          // Make sure mailbox is free
        HAL_CAN_AddTxMessage(bms.can, &header, tx.Data, &mailbox);  // Send frame
    }
}

static void txError(uint8_t id)
{
    // Locals
    CanTxMsgTypeDef tx;                                             // Tx frame to send

    tx.DLC = 2;                                                     // Send 2 bytes
    tx.IDE = CAN_ID_STD;                                            // Standard length ID
    tx.RTR = CAN_RTR_DATA;                                          // Data frame
    tx.StdId = ID_BMS_ERR;                                          // Error ID

    tx.Data[0] = 0x1;                                               // Set error
    tx.Data[1] = id;                                                // ID of LLC from which error came

    qSendToBack(&bms.q_rx_can_hlc, &tx);                            // Add frame to queue
}

void txCanHLC()
{
    // Locals
    CanTxMsgTypeDef     tx;                                             // Tx frame to send
    CAN_TxHeaderTypeDef header;                                         // Header frame for send
    uint32_t            mailbox;                                        // Mailbox for send

    if (qReceive(&bms.q_tx_can_hlc, &tx) == SUCCESS_G)                  // Check queue for items and take if there is one
    {
        header.DLC = tx.DLC;                                            // Copy frame length
        header.IDE = tx.IDE;                                            // Copy frame ID length
        header.RTR = tx.RTR;                                            // Copy frame type
        header.StdId = tx.StdId;                                        // Copy frame ID
        header.TransmitGlobalTime = DISABLE;                            // Don't send timestamp counter value

        while (!HAL_CAN_GetTxMailboxesFreeLevel(bms.can_hlc));          // Make sure mailbox is free
        HAL_CAN_AddTxMessage(bms.can_hlc, &header, tx.Data, &mailbox);  // Send frame
    }
}

void canProcess() {
    // Locals
    CanRxMsgTypeDef rx;                                             // Rx frame
    uint32_t        rx_comb;                                        // Buffer for combining bytes
    uint32_t        serial;                                         // Serial number for the IC

    if (qReceive(&bms.q_rx_can, &rx))                               // Check queue for items and take if there is one
    {
        if (bms.id == ID_HLC)                                       // Check if we are the HLC
        {
            serial = rx.Data[0];                                    // Capture ID of LLC sending voltage
            switch(rx.StdId)                                        // Run through all possible IDs
            {
                case ID_CELL_V:                                     // LLC is sending a set of voltage values
                    if (bms.id == 0)                                // Check if master accidentally sent voltages to itself
                    {
                        return;
                    }
                    // Pull 3 cell measurements out
                    bms.cells_con[serial - 1].chan_volts_raw[rx.Data[1]] = ((uint16_t) rx.Data[3]) | (((uint16_t) rx.Data[2]) << 8);
                    bms.cells_con[serial - 1].chan_volts_raw[rx.Data[1] + 1] = ((uint16_t) rx.Data[5]) | (((uint16_t) rx.Data[4]) << 8);
                    bms.cells_con[serial - 1].chan_volts_raw[rx.Data[1] + 2] = ((uint16_t) rx.Data[7]) | (((uint16_t) rx.Data[6]) << 8);
                    break;

                case ID_CELL_T:                                     // LLC is sending a set of temperature values
                    if (bms.id == 0)                                // Check if master accidentally sent temperatures to itself
                    {
                        return;
                    }
                    // Pull 3 temp measurement out
                    bms.cells_con[serial - 1].chan_temps_hlc[rx.Data[1]] = ((uint16_t) rx.Data[3]) | (((uint16_t) rx.Data[2]) << 8);
                    bms.cells_con[serial - 1].chan_temps_hlc[rx.Data[1] + 1] = ((uint16_t) rx.Data[5]) | (((uint16_t) rx.Data[4]) << 8);
                    bms.cells_con[serial - 1].chan_temps_hlc[rx.Data[1] + 2] = ((uint16_t) rx.Data[7]) | (((uint16_t) rx.Data[6]) << 8);
                    break;

                case ID_BMS_ERR:                                    // LLC has noticed an error in the system
                    if (rx.Data[0] == 0x1)                          // Check if an error has set on LLC
                    {
                        txError(rx.Data[1]);                        // Add error frame to HLC tx queue
                    }

                    // TODO: Unset errors when tx.Data[0] is 0 and pack in error ID

                    break;

                default:                                            // ID wasn't recognized
                    asm("nop");                                     // Do nothing so we can place a breakpoint
            }
        }
        else
        {
            switch(rx.StdId)
            {
                case ID_LLC_SELECT:                                 // HLC has responded with a new ID for (possibly) us
                    serial = HAL_GetREVID();                        // Get the serial number for the IC we're using
                    rx_comb = rx.Data[3];                           // Pack LSB
                    rx_comb |= ((uint16_t) rx.Data[2]) << 8;        // Pack second LSB
                    rx_comb |= ((uint32_t) rx.Data[1]) << 16;       // Pack second MSB
                    rx_comb |= ((uint32_t) rx.Data[0]) << 24;       // Pack MSB
                    if (serial == rx_comb)                          // Check if ID response is to us
                    {
                        bms.id = rx.Data[4];                        // Update our ID
                    }
                    break;

                case ID_HLC_RESPONSE:
                    // TODO: Implement response protocol for param update
                    break;

                default:                                            // ID wasn't recognized
                    asm("nop");                                     // Do nothing so we can place a breakpoint
            }
        }
    }
}

void can_filter_init(CAN_HandleTypeDef* hcan) {
    // Locals
    CAN_FilterTypeDef FilterConf;

    // TODO: Update filter IDs once CAN is set and enable
    FilterConf.FilterIdHigh =         0x00 << 5;                    // Set high ID
    FilterConf.FilterIdLow =          0x00 << 5;                    // Set low ID
    FilterConf.FilterMaskIdHigh =     0x00 << 5;                    // Set high mask
    FilterConf.FilterMaskIdLow =      0x00 << 5;                    // Set low mask
    FilterConf.FilterFIFOAssignment = CAN_FilterFIFO0;              // Set filter on FIFO0
    FilterConf.FilterBank = 0;                                      // Use filter bank 0
    FilterConf.FilterMode = CAN_FILTERMODE_IDMASK;                  // Use mask mode rather than ID mode
    FilterConf.FilterScale = CAN_FILTERSCALE_16BIT;                 // Use 16 bit filter
    FilterConf.FilterActivation = DISABLE;                          // Disable filter
    HAL_CAN_ConfigFilter(hcan, &FilterConf);                        // Configure filter
}
