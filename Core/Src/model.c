/*
 * model.c
 *
 * Created: 9/27/20
 */

#include "model.h"

/* TODO:
 * Track cell voltage over time so we can determine estimated balance current
 * Adjust module voltages based on output current
 */

// @funcname: init_accum()
//
// @brief: Initializes cell accumulators by loading models and prepping
//         initial values
void initAccum()
{
    // Locals
    uint8_t i;                                                      // Generic counter variable 

    // Trigger accumulator fault, let AFE init, and make a swag about capacity
}

// @funcname: track_accum()
//
// @brief: Integrates cell power to determine overcharge cases. This
//         directly connects to our ability to balance
//
// @exec_period: 200Hz or every 5 ms
void trackAccum()
{
    // Locals
    uint8_t           i;
    TickType_t        time_init;                                                                      // Loop time init
    float             delta;                                                                          // Amount of time passed between iterations
    static TickType_t time_prev;                                                                      // Previous iteration time

    time_init = xTaskGetTickCount();                                                                  // Gather current tick count
    delta = (time_init - time_prev) /  portTICK_PERIOD_MS;                                            // NOTE: THIS WON'T WORK!! RTOS TICK IS TOO SLOW
    time_prev = time_init;                                                                            // Store last entry time

    for (i = 0; i < bms.module_params.cells_series; ++i)                                              // Loop through each cell
    {
        bms.cells.est_cap[i] += bms.cells.mod_volts_conv * bms.pack_curr * delta;                     // Accumulate pack current (negative means discharge)
        bms.cells.est_cap[i] -= bms.cells.chan_volts_conv[i] * bms.cells.balance_current[i] * delta;  // Accumulate cell balance current (positive means discharge)
        bms.cells.est_cap[i] -= MCU_V * bms.mcu_current * delta;                                      // Accumulate main rail current (positive means discharge)
        bms.cells.est_cap[i] -= bms.cells.mod_volts_conv * bms.afe_current * delta;                   // Accumulate AFE current (positive means discharge)

        // There's a better way to do this so we don't set the flags every time. We'll squeeze out performance in the future
        if (bms.cells.est_cap[i] > BALANCE_THRESH)                                                    // Check if the cell is overcharged
        {
            bms.cells.balance_current[i] = bms.cells.chan_volts_conv[i] / BALANCE_RES;                // We are, so estimate a balance current...
            bms.cells.balance_flags |= 1 << i;                                                        // And set balance flag
        } else {
            bms.cells.balance_current[i] = 0;                                                         // We don't need to balance, so set current to 0...
            bms.cells.balance_flags &= ~(1 << i);                                                     // And reset balance flags
        }
        // While the data will be 5 ms stale, chemistry is slow, so we don't care that we don't update balance current before accumulating
    }
}