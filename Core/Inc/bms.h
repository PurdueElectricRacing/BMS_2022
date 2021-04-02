#ifndef _BMS_H_
#define _BMS_H_

// Includes
#include "main.h"
#include "can.h"
#include "afe.h"
#include "model.h"
#include "string.h"
#include "temp.h"
#include "eeprom.h"
#include "queue.h"
#include "fault_library.h"
#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_can.h"
#include "stm32l4xx_hal_uart.h"

// Generic Defines
#define GREAT           1                   // Great is 1, of course
#define PER             GREAT               // PER is great, of course

#define CELL_MAX        24                  // Maximum number of cells supported by platform (100 V limit)
#define TEMP_SUPPORT    24                  // Maximum number of temperature readings supported by platform (arbitrary)
#define MCU_V           4                   // Main rail voltage (should be near 4)
#define BALANCE_THRESH  XXXX                // Cell capacity limit
#define BALANCE_RES     25                  // Balance resistor value in ohms
#define FCLK            80                  // Fclk freuqency in MHz
#define CELL_UV_THRESH  3.0f                // Cell undervoltage threshold
#define CELL_OV_THRESH  4.3f                // Cell overvoltage threshold
#define NUM_TEMP        2                   // Number of thermistor ADCs
#define NUM_CHANNELS    16                  // Number of channels per thermistor ADC
#define RX_SIZE_UART    30                  // Max RX size
#define TX_SIZE_UART    10                  // Max TX size
#define ID_HLC          0                   // ID for the master PCB
#define LLC_MAX         6                   // Max number of LLCs the system supports

// Enumerations
//typedef enum {
//    FAILURE_G,                              // Failure = 0
//    SUCCESS_G                               // Success = 1
//} status_t;

typedef enum {
    FULL,                                   // BMS has full control
    REQUEST,                                // BMS has to request control
    NONE                                    // HLC has full control
} permissions_t;

typedef enum {
    BAUD_250_K,                             // 250 kb AFE baud rate
    BAUD_1_M                                // 1 Mb AFE baud rate
} afe_rate_t;

typedef enum {
    BOOT_HARD,                              // First time booting (cleared EEPROM)
    BOOT_SOFT,                              // Soft boot after debug
    BOOT_WFS,                               // Wake from sleep
    BOOT_WDAWG,                             // Watchdog reset
    BOOT_RESET,                             // User reset (GUI/button)
    BOOT_EEPROM_FAIL                        // EEPROM load failure
} boot_type_t;

typedef enum {
    LLC,                                    // Low level controller
    HLC                                     // High level controller
} board_type_t;

typedef enum {
    CELL_21_2021,
    CELL_21_2020
} board_layout_t;

enum {
    TASK_1_MS,                              // 1 ms loop task
    TASK_5_MS,                              // 5 ms loop task
    TASK_100_MS,                            // 100 ms loop task
    TASK_BG,                                // Background loop task
    TASK_COUNT                              // Number of RTOS tasks
};

// Structures
typedef struct {
    uint8_t  rx_buffer[RX_SIZE_UART];       // RX buffer
    uint16_t rx_size;                       // RX size
} uart_rx_t;

typedef struct {
    uint8_t  tx_buffer[TX_SIZE_UART];       // TX buffer
    uint16_t tx_size;                       // TX size
} uart_tx_t;

typedef struct {
    board_type_t board;                     // Determines if PCB is in HLC or LLC mode
    uint8_t      cells_series;              // Number of cells in series per module
} personality_t;

typedef struct {
    permissions_t   perms;                  // Current module permissions
    board_layout_t  layout;                 // Setup of PCB for any specific nuances
    uint32_t        eeprom_size;            // Size for the EEPROM IC on a given PCB
    uint8_t         cells_series;           // Number of cells in series
    uint8_t         temp_ic;                // Deprecated value for number of temp ICs
    uint8_t         temp_chan;              // Deprecated value for number of channels per IC
} params_t;

typedef struct {
    boot_type_t current_boot;               // Current boot status for the MCU
    boot_type_t historic_boot;              // Boot type last time (used to catch hard faults)
} boot_historic_t;

typedef struct {
    uint32_t task_entry_time;       // Tick time of task entry
    uint32_t bg_entry_time;         // Tick time of background entry
    uint16_t task_time;             // Amount of time spent in a task
    uint16_t bg_time;               // Amount of time spent in the background loop
    float    cpu_use;               // % use of task time (TODO: update to be true cpu %)
} cpu_use_t;

typedef struct {
    uint8_t  skips;                 // Number of times the scheduler updated next call without proper execution. Anything other than 0 is an issue
    uint8_t  next_idx;              // Loop index for next function to call
    uint8_t  run_next;              // Triggers background to run the next iteration
    uint16_t fos;                   // Frequency of scheduler
    uint32_t os_ticks;              // Current tick count for timer 14

    cpu_use_t core;                 // Timing information
} scheduler_t;

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
    uint16_t chan_temps[NUM_TEMP][NUM_CHANNELS];
    uint16_t chan_temps_hlc[NUM_TEMP * NUM_CHANNELS * LLC_MAX];
} cells_t;

typedef struct {
    uint32_t balance_force;                 // Forced balance flags
} manual_override_t;

typedef struct {
    uint16_t   vm_raw;                      // Raw 14 bit ADC value for internal -5 V charge pump output
    float      vm_conv;                     // Converted voltage value
    uint16_t   v5vao_raw;                   // Raw 14 bit ADC value for 5 V always on rail
    float      v5vao_conv;                  // Converted voltage value
    uint16_t   vdig_raw;                    // Raw 14 bit ADC value for digital I/O rail
    float      vdig_conv;                   // Converted voltage value
    uint16_t   vdd18_raw;                   // Raw 14 bit ADC value for internal 1.8 V digital supply
    float      vdd18_conv;                  // Converted voltage value

    afe_rate_t comm_rate;                   // Current set baud rate (250 kb on initial boot)
} afe_t;

typedef struct {
    CAN_HandleTypeDef*  can;                // CAN Handle
    CAN_HandleTypeDef*  can_hlc;            // CAN Handle (for HLC to car)
    UART_HandleTypeDef* uart;               // UART Handle

    q_handle_t          q_rx_can;           // CAN RX queue
    q_handle_t          q_tx_can;           // CAN TX queue

    q_handle_t          q_rx_can_hlc;       // CAN RX queue (for HLC to car)
    q_handle_t          q_tx_can_hlc;       // CAN TX queue (for HLC to car)

    afe_t               afe;                // AFE voltage readings and parameters

    params_t            module_params;      // Module parameters
    boot_historic_t     boot_stat;          // Module boot type
    cells_t             cells;              // Module cell data
    cells_t             cells_con[LLC_MAX]; // Conglomeration of all cells in the system (excluding those on HLC)
    manual_override_t   override;           // User configurable overrides

    uint8_t             id;                 // PCB ID

    float               mcu_current;        // Current draw of MCU and surrounding devices
    float               afe_current;        // Current draw of AFE
} bms_t;

extern bms_t bms;                           // Global BMS structure
extern scheduler_t scheduler;               // Global "scheduler" structure

// Prototypes
void init_RTOS_objs();                      // Initializes queues and RTOS tasks
void init_bms();                            // Initializes BMS
void task_1ms_loop();                       // 1 ms loop
void task_5ms_loop();                       // 5 ms loop
void task_100ms_loop();                     // 100 ms loop
void task_bg();                             // Background loop
void initScheduler();                       // Initializes scheduler
void startTasks();                          // Sets timer 14 to run and start loop
void pauseTasks();                          // Stops timer 14 to pause looop

#endif
