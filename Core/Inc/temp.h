#ifndef _TEMP_H_
#define _TEMP_H_

// Includes
#include "bms.h"
#include <math.h>

// Defines
#define NUM_CHANNELS 16

#define ID_TEMP_1   			0b1000110
#define ID_TEMP_2   			0b1000101
#define ID_TEMP_GLOBAL			0b1110111
#define TRIALS                  2
#define NUM_CHANNELS            16
#define ACQUIRE_TEMP_STACK_SIZE 128
#define ACQUIRE_TEMP_PRIORITY   1
#define I2C_TIMEOUT             100
#define READ_REQ_WAIT			200
#define WRITE_REQ_WAIT			200
#define ACQUIRE_TEMP_RATE       500

// Writing
#define WRITE_MSG_SIZE    		2
#define WRITE_ENABLE      		0x00 //xxxx-xxx0
#define CHANGE_CHANNEL    		0xA0 //101x-xxxx
#define WRITE_TIMEOUT			1000

#define SAME_CHANNEL            0x00 //000x-xxxx

#define SGL_SHIFT               4
#define SGL_MASK                0x10  //xxx1-xxxx
#define SGL_DIFF                0
#define SGL_SINGLE              1

#define CHANNEL_0               0x00 //xxxx-0000
#define CHANNEL_1               0x08 //xxxx-1000
#define CHANNEL_2               0x01 //xxxx-0001
#define CHANNEL_3               0x09 //xxxx-1001
#define CHANNEL_4               0x02 //xxxx-0010
#define CHANNEL_5               0x0A //xxxx-1010
#define CHANNEL_6               0x03 //xxxx-0011
#define CHANNEL_7               0x0B //xxxx-1011

#define CHANNEL_8               0x04 //xxxx-0100
#define CHANNEL_9               0x0C //xxxx-1100
#define CHANNEL_10              0x05 //xxxx-0101
#define CHANNEL_11              0x0D //xxxx-1101
#define CHANNEL_12              0x06 //xxxx-0110
#define CHANNEL_13              0x0E //xxxx-1110
#define CHANNEL_14              0x07 //xxxx-0111
#define CHANNEL_15              0x0F //xxxx-1111

// Reading
#define READ_ENABLE             0x01 //xxxx-xxx1
#define READ_MSG_SIZE           3    //24 bits sent per msg

//T hermistor Constants NXRT15XV103FA1B
#define B_VALUE                 3977      // Beta value 25*C to 85*C temp range
#define THERM_RESIST            10000     // 10K Ohm resistor
#define THERM_TAU               4         // 4 second time constant
#define THERM_DISS_CONST        1.5       // 1.5 second thermal dissipation constant
#define VOLTAGE_REF             2.5       // Voltage Ref into LTC2497 is 5 volts (needs to change)
#define AMBIENT_TEMP            298.15    // Degrees C
#define VOLTAGE_TOP             5         // Top of the voltage divider
#define R_INF_3977              0.016106  // r_inf = R0 * exp(-Beta/t0) beta = 3977
#define KELVIN_2_CELSIUS        273.15

// Macros
#define channel_combine(channel) CHANGE_CHANNEL | SGL_MASK | channel
#define set_address(address, write_en) (address << 1) | write_en

// Prototypes
void task_acquire_temp();
uint16_t readLTCValue(uint8_t currentChannel, uint8_t tap);


#endif