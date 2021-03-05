#include "idRequest.h"

/*
    Theory of operation:

    Check if we have an ID assigned to us.
    If we do, great, load the personality for that PCB.
    If not, talk to master to see if we can get an ID assigned to us.

    Assignment process:
    Generate a true random number.
    Send that random number in a CAN frame to the master board.
    Master then looks at the number of LLCs on the bus. It sends back an ID.
    That ID will be the number of LLCs + 1, thus, incrementing the number of LLCs on the bus.
*/

static void getPersonality()
{
    // Locals
    CanTxMsgTypeDef tx;                 // Tx struct
    uint16_t        rand;               // Random number

    if (bms.id != 0xff)
    {
        // Something went wrong with the process and an ID is already assigned
        // This function should not have been called
    }

    // TODO: Configure true RNG hardware
    RNG->RNG_CR |= RNGEN;               // Enable random number generator
    while (RNG->RNG_SR & )

    // TODO: Gather a random value

    tx.StdId = ID_LLC_SELECT;           // Set ID to let HLC know we need an ID
    tx.IDE = CAN_ID_STD;                // Set to standard length
    tx.RTR = CAN_RTR_DATA;              // Set to data frame
    tx.DLC = 2;                         // Set data length to 2 bytes
}

// Checks to see if we have a valid ID assigned to us. If not, it gets one for us.
// This function 
void checkPersonality()
{
    if (bms.id == 0xff)                 // Check if the ID is outside bounds
    {
        getAssignment();                // Request a LLC ID
    }

    // TODO: Read ADC values

    // TODO: Assign personality
}