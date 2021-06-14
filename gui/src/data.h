#ifndef _DATA_H_
#define _DATA_H_

// Includes
#include <gtk-3.0/gtk/gtk.h>
#include "main.h"
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

// Defines
#define CELL_COUNT      21
#define MODULE_COUNT    5

// Structures
typedef struct {
    bool     con_stat;
    uint8_t  con_type;
    bool     act_stat;
    uint8_t  mod_config;
    uint8_t  hrev;
    uint8_t  srev;
    uint16_t overrides;
    uint32_t mod_volts;
    bool     eeprom_stat;
    bool     global_bal;
    bool     mod_params;
    uint8_t  fan_speed;
    uint16_t min_volts;
    uint16_t max_volts;
    uint16_t avg_volts;
    uint16_t min_temp;
    uint16_t max_temp;
    uint16_t avg_temp;
} module_t;

typedef struct {
    uint16_t volts[CELL_COUNT];
    uint16_t voltsb[CELL_COUNT];
    uint16_t temps[CELL_COUNT];
    uint16_t tempsb[CELL_COUNT];
    uint16_t current[CELL_COUNT];
    bool     bal[CELL_COUNT];
} cells_t;

typedef struct {
    bool status[CELL_COUNT];
    bool historic[CELL_COUNT];
    bool override[CELL_COUNT];
} faults_t;

typedef struct {
    uint8_t timing;
    uint8_t misses;
    uint8_t jitter;
    uint16_t mcu_current;
    uint16_t afe_current1;
    uint16_t afe_current2;
    uint16_t v5ao;
    uint16_t vm;
    uint16_t vdig;
    uint16_t vdd18;
    uint16_t afe_rate;
} debug_t;

// Function prototypes
void dataInit(app_widgets_usr* wdgts);
gboolean bgLoop(void* nothing);

#endif