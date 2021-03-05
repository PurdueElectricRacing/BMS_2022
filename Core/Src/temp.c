/*
 * temp_adc.c
 *
 *  Created on: Feb 10, 2019
 *      Author: Matt Flanagan & Dawson Moore
 */
#include "temp.h"

void init_LTC2497(uint8_t * write_data);
uint16_t adc2temp(uint16_t adc_val);
inline uint16_t adc_extract(uint8_t * arr);
uint16_t adc2temp(uint16_t adc_val);

uint8_t temp_array[READ_MSG_SIZE];
uint16_t adc_val, adc_val0, adc_val1, adc_val2, adc_val3, adc_val4;
const uint8_t channel[NUM_CHANNELS] = {CHANNEL_0, CHANNEL_1,
                                       CHANNEL_2, CHANNEL_3, CHANNEL_4, CHANNEL_5, CHANNEL_6,
                                       CHANNEL_7, CHANNEL_8, CHANNEL_9, CHANNEL_10, CHANNEL_11,
                                       CHANNEL_12, CHANNEL_13, CHANNEL_14, CHANNEL_15
                                      };

const uint8_t chip[2] = {ID_TEMP_1, ID_TEMP_2};
uint8_t read_byte = 0;
uint8_t write_data[WRITE_MSG_SIZE + 1];

void task_acquire_temp() {
	uint8_t i, tap, faultCount;
	uint16_t temperature;

	init_LTC2497(write_data);

	TickType_t time_init = 0;
	while (PER == GREAT)
	{
	  for (i = 0; i < NUM_TEMP; ++i)
	  {
		  //Set the next channel to read for both ICs and start conversion
		  write_data[1] = channel_combine(channel[i]);
		  HAL_I2C_Master_Transmit(&hi2c1, (uint16_t) write_data[0], &write_data[1], WRITE_MSG_SIZE, 0xFFFF);
		  while (hi2c1.State != HAL_I2C_STATE_READY)
		  {
			  //Wait for the send to stop
		  }
		  vTaskDelay(WRITE_REQ_WAIT);
		  for (tap = 0; tap <= 1; ++tap)
		  {
			  temperature = readLTCValue(i, tap);

			  if (xSemaphoreTake(bms.temp.sem, TIMEOUT) == pdTRUE)
			  {
				  if (temperature <= 0 || temperature >= 1000 )
				  {
					  faultCount = 0;
					  //Probably a bad sensor reading
					  do {
						  temperature = readLTCValue(i, tap); //Retake the measurement until we get a good value
					  } while (temperature <= 10 && temperature >= 100 && faultCount++ < 10); //Try 10 times
					  if (faultCount == 10) //We tried 10 times and it still doesn't work
					  {
						  if (tap == 1)
						  {
							  bms.temp1_con = FAILURE;
						  } else
						  {
							  bms.temp2_con = FAILURE;
						  }
					  } else
					  {
						  bms.temp.data[tap][i] = temperature;
					  }
				  } else
				  {
					  bms.temp.data[tap][i] = temperature;
				  }
				  xSemaphoreGive(bms.temp.sem);
			  } else
			  {
				  //Error
			  }
		  }
	  }
	}
	vTaskDelayUntil(&time_init, ACQUIRE_TEMP_RATE);
}

uint16_t readLTCValue(uint8_t currentChannel, uint8_t tap)
{
	//Set the next channel to read for both ICs and start conversion
	write_data[1] = channel_combine(channel[currentChannel]);
	HAL_I2C_Master_Transmit(&hi2c1, (uint16_t) write_data[0], &write_data[1], WRITE_MSG_SIZE, 0xFFFF);
	while (hi2c1.State != HAL_I2C_STATE_READY)
	{
	  //Wait for the send to stop
	}
	HAL_Delay(WRITE_REQ_WAIT);
	read_byte = set_address(chip[tap], READ_ENABLE);
	HAL_I2C_Master_Receive(&hi2c1,(uint16_t) read_byte, &temp_array[0], READ_MSG_SIZE, 0xFFFF);
	while (hi2c1.State != HAL_I2C_STATE_READY)
	{
	  //Wait for the send to stop
	}
	adc_val = adc_extract(temp_array);

	return(adc2temp(adc_val));
}

inline uint16_t adc_extract(uint8_t * arr)
{
	return ((uint16_t) arr[0] << 10) | ((uint16_t) arr[1] << 2) | ((uint16_t) arr[2] >> 6);                 // Returned extracted ADC value
}

uint16_t adc2temp(uint16_t adc_value)
{
    // Locals
	float voltage;                                                                                          // Estimated thermistor voltage
	float thermistor_res;                                                                                   // Estimated thermistor resistance
	double temperature;                                                                                     // Estimated thermistor temperature
    
	//calculate the voltage from the adc_val
	voltage = VOLTAGE_REF * ((float) (adc_value)) / 0xFFFF;                                                 // Calculate the voltage from the ADC value
	thermistor_res = (voltage * THERM_RESIST) / (VOLTAGE_TOP - voltage);                                    // Calculate the resistance from the voltage
	temperature =  B_VALUE / log (thermistor_res / R_INF_3977) - KELVIN_2_CELSIUS;                          // Calculate the temperature

	return (uint16_t)(10 * temperature);
}

void initLTC()
{
	uint8_t timeout;                                                                                        // Timeout counter
    uint8_t i;                                                                                              // Loop counter variable
    uint8_t write_data[3];                                                                                  // Data to write

    for (i = 0; i < 2; i++)
    {
        timeout = 0;                                                                                        // Set the timeout to 0 for this loop
        write_data[0] = set_address(chip[i], WRITE_ENABLE);                                                 // Set address to chip i
        write_data[1] = channel_combine(channel[0]);                                                        // Setup channel byte
        write_data[2] = 0xFF;                                                                               // Write wakeup
        HAL_I2C_Master_Transmit(&hi2c1, (uint16_t) write_data[0], &write_data[1], WRITE_MSG_SIZE, 0xFFFF);  // Transmit wakeup

        while (hi2c1.State != HAL_I2C_STATE_READY && timeout++ < WRITE_TIMEOUT);                            // Wait until I2C enters ready state
        if (timeout >= WRITE_TIMEOUT)                                                                       // Check if we didn't write properly
        {
            // TODO: Use fault library for this
            bms.temp_con = FAULTED;                                                                         // Set temperature connection status to fault
        }
        HAL_Delay(WRITE_REQ_WAIT);                                                                          // Wait the required time period
    }
}