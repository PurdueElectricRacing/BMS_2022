/*
 * boot.c
 *
 * Created: 2/17/20
 */

#include "boot.h"

// @funcname: initEEPROM()
//
// @brief: Initializes EEPROM library, attempts to load parameters.
//         If this is the first boot, default paramemters will be written.
void initEEPROM()
{
    bms.boot_stat.current_boot = BOOT_EEPROM_FAIL;                      // Initialize value
    bms.boot_stat.historic_boot = BOOT_EEPROM_FAIL;                     // Initialize value
    eInitialize(&hi2c1, bms.module_params.eeprom_size, START_ADDRESS);  // Attempt to initialize the EEPROM
    eLinkStruct(&bms.boot_stat, sizeof(bms.boot_stat), "BTH", 1, 0);    // Attempt to link boot struct so we can see what our boot status is
    eCleanHeaders();                                                    // Clean headers
    eLoadStruct("BTH");                                                 // Load boot struct if it exists
}

// @funcname: validateBoot()
//
// @brief: Checks current and historic boot status to ensure the boot is valid.
//         If the boot is invalid, send critical error, else continue with loading.
//
// @return: SUCCESS if valid, FAILURE else
status_t validateBoot()
{
    // Local variables
    status_t ret = FAILURE;                                             // Initialize to failure state
    CanTxMsgTypeDef tx;                                                 // CAN TX frame

    // Check to see if we successfully overwrote the default values during EEPROM load
    if (bms.boot_stat.current_boot == BOOT_EEPROM_FAIL || bms.boot_stat.historic_boot == BOOT_EEPROM_FAIL)
    {
        bms.boot_stat.current_boot = BOOT_HARD;                         // Save as first time boot
        bms.boot_stat.historic_boot = BOOT_HARD;                        // Save as first time boot
        eSaveStruct("BTH");                                             // Write current boot values

        ret = SUCCESS;                                                  // Successful first run
    }
    else if (bms.boot_stat.historic_boot == BOOT_WDAWG)                 // Check if powering on from watchdog reset
    {
        
    }
}