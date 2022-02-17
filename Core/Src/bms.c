#include "bms.h"

// Static function prototpyes
static void memsetu(uint8_t* ptr, uint8_t val, uint32_t size);

bms_t bms;

// @funcname: bmsStatus
//
// @brief: Posts BMS status
void bmsStatus()
{
    static uint8_t flag;

    if (bms.error != 0)
    {
        flag = !flag;
    }
    else
    {
        flag = 0;
    }

    HAL_GPIO_TogglePin(HEARTBEAT_GPIO_Port, HEARTBEAT_Pin);
    HAL_GPIO_WritePin(ERR_LED_GPIO_Port, ERR_LED_Pin, flag);
    HAL_GPIO_WritePin(CONN_LED_GPIO_Port, CONN_LED_Pin, bms.afe_con);
}

// @funcname: initBMS
//
// @brief: Initializes BMS parameters and scheduler
void initBMS()
{
    memsetu((uint8_t*) &bms, 0, sizeof(bms));
    while (afeInit() == HAL_ERROR) {
        HAL_GPIO_WritePin(ERR_LED_GPIO_Port, ERR_LED_Pin, 1);
        HAL_GPIO_WritePin(HEARTBEAT_GPIO_Port, HEARTBEAT_Pin, 0);
    }
    
    schedInit(80000000);
    taskCreate(bmsStatus, 500);
    taskCreate(afeTask, 100);
    schedStart();
}

// @funcname: memsetu
//
// @brief: Simple memset routine
//
// @param: ptr: Pointer to location to set
// @param: val: Value to set each memory address to
// @param: size: Length of data to set
static void memsetu(uint8_t* ptr, uint8_t val, uint32_t size)
{
    // Locals
    uint32_t i;

    for (i = 0; i < size; i++)
    {
        ptr[i] = val;
    }
}
