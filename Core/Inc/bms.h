#ifndef _BMS_H_
#define _BMS_H_

// Includes
#include "main.h"
#include "afe.h"

// Generic Defines
#define GREAT       1
#define PER         GREAT
#define CELL_MAX    24
#define TEMP_MAX    CELL_MAX / 2

// Structures
typedef struct {
    // Cells are indexed from bottom of stack to top of stack
    uint16_t chan_volts_raw[CELL_MAX];      // Raw 14 bit ADC value for each cell's voltage
    float    chan_volts_conv[CELL_MAX];     // Converted voltage values
    uint16_t mod_volts_raw;                 // Raw 14 bit ADC value for module
    float    mod_volts_conv;                // Converted voltage value
    float    est_cap[CELL_MAX];             // Current estimated cell capacity in W*hr
    float    est_cap_max[CELL_MAX];         // Current estimated maximum cell capacity in W*hr
    float    est_SOC[CELL_MAX];             // Current estimated cell state of charge in %
    float    est_SOH[CELL_MAX];             // Current estimated cell state of health in %
    float    pack_current;                  // Current being pulled from the pack
    float    balance_current[CELL_MAX];     // Current from balance circuit for each cell

    uint32_t balance_flags;                 // Cell overcharge flag
    uint32_t balance_mask;                  // Cell balancing flag masks

    // Converted temperature values
    uint16_t chan_temps[TEMP_MAX];
} cells_t;

typedef struct {
    uint32_t error;
    uint8_t  afe_con;
    cells_t  cells;
} bms_t;

extern bms_t bms;                           // Global BMS structure

// Prototypes
void bmsStatus();
void initBMS();

#endif