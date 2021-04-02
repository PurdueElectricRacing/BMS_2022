/*
 * fault_library.h
 *
 *  Created on: Feb 6, 2020
 *      Author: Luke Oxley
 */

#ifndef FAULT_LIBRARY_H_
#define FAULT_LIBRARY_H_

#include <string.h>
#include "main.h"
#include "stm32l4xx_hal.h"
#include "eeprom.h"

// Be sure to include headers with locations to functions!
#include "bms.h"

#define PER 1
#define GREAT PER

#define FAULT_LIB_PERIOD 1 //ms

#define FAULT_MAX 32

#define FAULT_EEPROM_NAME "flt"

// GENERATED VALUES ------
typedef enum { OVERVOLTAGE_FAULT_NUM, UNDERVOLTAGE_FAULT_NUM, OVER_SOC_FAULT_NUM, UNDER_SOC_FAULT_NUM, AFE1_CONNECTION_FAULT_NUM, AFE2_CONNECTION_FAULT_NUM, TEMP_IMPLAUS_FAULT_NUM, TEMP_HIGH_FAULT_NUM, TEMP_CONNECTION_FAULT_NUM, NO_IDX_FAULT_NUM, MASTER_DISCONNECT_FAULT_NUM, SLAVE_DISCONNECT_FAULT_NUM } fault_name_t;
#define HISTORIC_INIT 0x0
#define ENABLE_INIT 0xfff
#define CRITICALITY_INIT 0xaaaaaa
// END GENERATED VALUES ---------

typedef struct {
  uint32_t signal; //reset after each checking cycle
  uint32_t set; //achieved minimum time
  uint32_t historic; //only resets by user
  uint64_t criticality; //2 bit bit field
} fault_stored_t;

typedef struct {
  volatile fault_stored_t stored;
  uint16_t cycle_count[FAULT_MAX]; // cycles since first signal
  uint16_t signal_count[FAULT_MAX]; // signals seen since first signal
  uint32_t historic_type;
  uint32_t enable_type;
} fault_t;

fault_t faults;

typedef enum {
  HISTORIC_IGNORE,
  HISTORIC_OVERRIDE
} fault_historic_t;

typedef enum {
  FAULT_WARNING,
  FAULT_ERROR,
  FAULT_CRITICAL
} fault_criticality_t;

typedef enum {
  FAULT_DISABLED,
  FAULT_ENABLED
} fault_enable_t;

void faultLibInitialize();
void faultLibUpdate();
void signalFault(uint8_t loc);
void faultLibShutdown();
void clearHistory();
uint8_t getFaultSet(uint8_t loc);
uint8_t getFaultSignal(uint8_t loc);
uint8_t getHistoricOverriding(uint8_t loc);
fault_criticality_t getCriticality(uint8_t loc);
fault_enable_t getFaultEnabled(uint8_t loc);

//Generic criticality functions
//Called once when fault set
void handleCriticalFault();
void handleErrorFault();
void handleWarningFault();

#endif
