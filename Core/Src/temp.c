/*
 * temp_adc.c
 *
 *  Created on: Feb 10, 2019
 *      Author: Matt Flanagan & Dawson Moore
 */

#include "temp.h"

// LOCAL DEFNES
#define NUM_TEMP_CHANNELS			(16U)
#define NUM_DEVICES                 (2U)
#define WAIT_TIME_MS                (200U)

#define I2C_READ_ENABLE             (0x01)             // xxxx-xxx1
#define I2C_READ_MSG_SIZE           (NUM_CHANNELS * 2) // 2 bytes per temp reading
#define READ_MSG_SIZE				(16 * 2)

// Local data
static uint8_t const I2C_ADDRS[NUM_DEVICES] = {
    ((0x10 << 1) | I2C_READ_ENABLE),
    ((0x20 << 1) | I2C_READ_ENABLE),
};

// Local typedefs
typedef enum {
    WAIT,
    CHANNEL_UPDATE,
} temp_state_t;

// @funcname: acquireTemp
//
// @brief: Gathers temperature values for all ICs and all channels requested by user
//
// @exec_period: 1000Hz or every 1 ms
void acquireTemp()
{
    // Locals
    static uint8_t      ic         = 0;     // Current ic being read
    static uint8_t      wait       = 0;     // Current wait time
    static temp_state_t state      = WAIT;
    static temp_state_t nextState  = WAIT;

    nextState = state;
    switch(state)
    {
        case WAIT:
            ++wait;
            if (WAIT_TIME_MS <= wait)
            {
                wait = 0;
                nextState = CHANNEL_UPDATE;
            }
            break;

        case CHANNEL_UPDATE:
            // Read ADC vals into buffer
            HAL_I2C_Master_Receive(
                &hi2c1,(uint16_t) 
                I2C_ADDRS[ic],
                (uint8_t*) &(bms.cells.chan_temps[ic * NUM_TEMP_CHANNELS]),
                READ_MSG_SIZE, 
                0xFFFF);

            ++ic;
            if (NUM_CHANNELS <= ic)
            {
                ic = 0;
                nextState = WAIT;
            }
            break;
        default:
            nextState = WAIT;
            break;
    }

    state = nextState;

}

// @funcname: tempConnectionFaultSet
//
// @brief: Handles a temp connection fault
void tempConnectionFaultSet()
{
	// TODO: Handle temp connection fault (set)
}
