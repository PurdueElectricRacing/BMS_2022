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

// @funcname: setBalance
//
// @brief: Sets balance flag and updates estimated balance current
//
// @param: i: Cell index to modulate
static void setBalance(uint8_t i)
{
    bms.cells.balance_current[i] = bms.cells.chan_volts_conv[i] / BALANCE_RES;                          // Estimate a balance current...
    bms.cells.balance_flags |= 1U << i;                                                                 // And set balance flag
}

// @funcname: clearBalance
//
// @brief: Clears balance flag and updates estimated current to 0
//
// @param: i: Cell index to modulate
static void clearBalance(uint8_t i)
{
    bms.cells.balance_current[i] = 0;                                                                   // Set estimated balance current to 0...
    bms.cells.balance_flags &= ~(1 << i);                                                               // And reset balance flags
}

// @funcname: init_accum()
//
// @brief: Initializes cell accumulators by loading models and prepping
//         initial values. Also sets up free running timer for delta
//         timings.
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
    uint32_t          time_init;                                                                        // Loop time init
    float             delta;                                                                            // Amount of time passed between iterations
    static uint32_t   time_prev;                                                                        // Previous iteration time

    // TODO: Remove RTOS stuff

    time_init = TIM2->CNT;                                                                              // Gather current tick count
    delta = (time_init - time_prev) /  1000;                                                            // Calculate seconds since last run
    time_prev = time_init;                                                                              // Store last entry times

    for (i = 0; i < bms.module_params.cells_series; i++)                                                // Loop through each cell
    {
        if (bms.override.balance_force & (1U << i))                                                     // CASE 0: User requests an override
        {
            setBalance(i);                                                                              // Flag cell as requiring balancing
        }
        else if (bms.cells.balance_mask & (1U << i))                                                    // CASE 1: Cell is masked for some fault condition
        {
            clearBalance(i);                                                                            // Clear balance flag
        }
        else if (bms.cells.chan_volts_conv[i] > CELL_UV_THRESH)                                         // CASE 2: Cell has an overvolt condition
        {
            setBalance(i);                                                                              // Flag cell as requiring balancing
            signalFault(OVERVOLTAGE_FAULT_NUM);
        }
        else if (bms.cells.est_SOC[i] > 100)                                                            // CASE 3: Cell has a high SOC event
        {
            setBalance(i);                                                                              // Flag cell as requiring balancing
            signalFault(OVER_SOC_FAULT_NUM);
        }
        // TODO: Implement model
//        else if ((bms.cells.est_cap[i] > bms.cells.est_cap_max) && TRACK_CAP)                           // CASE 4: Cell has an estimated capacity higher than what SOH says is possible
//        {
//            setBalance(i);                                                                              // Flag cell as requiring balancing
//        }
//        else if (bms.cells.est_SOC[i] > bms.cells.avg_SOC + SOC_THRESH)                                 // CASE 5: Cell has a much higher SOC than other cells
//        {
//            setBalance(i);                                                                              // Flag cell as requiring balancing
//            signalFault(OVER_SOC_FAULT_NUM); // TODO: same fault as high SOC event?
//        }
        else                                                                                            // CASE 6: Cell is within limits
        {
            clearBalance(i);                                                                            // Clear balance flag
        }

        bms.cells.est_cap[i] += bms.cells.mod_volts_conv * bms.cells.pack_current * delta;              // Accumulate pack current (negative means discharge)
        bms.cells.est_cap[i] -= bms.cells.chan_volts_conv[i] * bms.cells.balance_current[i] * delta;    // Accumulate cell balance current (positive means discharge)
        bms.cells.est_cap[i] -= MCU_V * bms.mcu_current * delta;                                        // Accumulate main rail current (positive means discharge)
        bms.cells.est_cap[i] -= bms.cells.mod_volts_conv * bms.afe_current * delta;                     // Accumulate AFE current (positive means discharge)
    }

    // While the data will be 5 ms stale, chemistry is slow, so we don't care that we don't update balance current before accumulating
}

// @funcname: overvoltageFaultSet
//
// @brief: Handles an overvoltage fault
void overvoltageFaultSet()
{
    // TODO: overvoltage fault set
}

// @funcname: overSOCFaultSet
//
// @brief: Handles an over SOC fault
void overSOCFaultSet()
{
    // TODO: over SOC fault set
}
