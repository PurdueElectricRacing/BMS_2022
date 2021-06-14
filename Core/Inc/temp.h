#ifndef _TEMP_H_
#define _TEMP_H_

// Includes
#include "bms.h"
#include <math.h>

// Externs
extern I2C_HandleTypeDef hi2c1;
extern bms_t bms;
// Prototypes
void acquireTemp();

#endif
