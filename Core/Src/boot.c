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
    bms.boot_stat.current_boot = BOOT_EEPROM_FAIL;                                      // Initialize value
    bms.boot_stat.historic_boot = BOOT_EEPROM_FAIL;                                     // Initialize value
    eepromInitialize(&hi2c1, bms.module_params.eeprom_size, EEPROM_I2C_ADDRESS);        // Attempt to initialize the EEPROM
    eepromLinkStruct(&bms.boot_stat, sizeof(bms.boot_stat), "BTH", 1, 0);               // Attempt to link boot struct so we can see what our boot status is
    eepromLinkStruct(&faults.stored, sizeof(faults.stored), FAULT_EEPROM_NAME, 1, 1);   // Link fault library struct in eeprom
    eepromCleanHeaders();                                                               // Clean headers
    eepromLoadStruct("BTH");                                                            // Load boot struct if it exists

    // TODO: faultLibInitialize();

}

// @funcname: validateBoot()
//
// @brief: Checks current and historic boot status to ensure the boot is valid.
//         If the boot is invalid, send critical error, else continue with loading.
//
// @return: SUCCESS if valid, FAILURE else
success_t validateBoot()
{
    // Local variables
    success_t ret = FAILURE_G;                                          // Initialize to failure state
    CanTxMsgTypeDef tx;                                                 // CAN TX frame

    // Check to see if we successfully overwrote the default values during EEPROM load
    if (bms.boot_stat.current_boot == BOOT_EEPROM_FAIL || bms.boot_stat.historic_boot == BOOT_EEPROM_FAIL)
    {
        bms.boot_stat.current_boot = BOOT_HARD;                         // Save as first time boot
        bms.boot_stat.historic_boot = BOOT_HARD;                        // Save as first time boot
        eepromSaveStruct("BTH");                                        // Write current boot values

        ret = SUCCESS_G;                                                // Successful first run
    }
    else if (bms.boot_stat.historic_boot == BOOT_WDAWG)                 // Check if powering on from watchdog reset
    {
        
    }

    return ret;
}
