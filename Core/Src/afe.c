#include "afe.h"

// Static function prototypes
static void afeStartComm();
static void afeEndComm();
static uint16_t calculatePEC(uint8_t* data, uint8_t len);
int verifyPEC(uint8_t* data, uint16_t len, uint16_t PEC);

int afeInit()
{
    uint16_t voltage_low = VUV(1);
	uint16_t voltage_high = VOV(4.2);
	uint8_t cmd[LTC6811_REG_SIZE] = {0b00000101, voltage_low & 0xFF, (((voltage_high & 0xF) << 4) & 0xF0) | (((voltage_low >> 8) & 0xF) & 0x0F), (voltage_high >> 4) & 0xFF, 0, 0};
	uint8_t register_check[LTC6811_REG_SIZE] = {0};

	afeWakeup();
	broadcastWrite(WRCFGA, LTC6811_REG_SIZE, cmd);
	broadcastRead(RDCFGA, LTC6811_REG_SIZE, register_check);

	// Ensure that configuration register is as expected
	for (int i = 0; i < LTC6811_REG_SIZE; i++)
    {
		if (register_check[i] != cmd[i])
        {
            bms.error = 1;
			return HAL_ERROR;
        }
    }

    bms.afe_con = 1;
    bms.error = 0;

	broadcastPoll(DIAGN);

	return HAL_OK;
}

void afeTask()
{
    uint8_t  i, x;
    uint8_t  data[8];
    uint16_t valid_PEC;

    broadcastPoll(ADCVSC(2, DISCHARGE_NOT_PERMITTED));

    for (i = 0; i < 1; i++)
    {
        valid_PEC = broadcastRead(readCmd[i], LTC6811_REG_SIZE, data);

        x = i * 3;
        bms.cells.chan_volts_raw[x++] = byte_combine(data[1], data[0]);
        bms.cells.chan_volts_raw[x++] = byte_combine(data[3], data[2]);
        bms.cells.chan_volts_raw[x] = byte_combine(data[5], data[4]);
    }
}

// @funcname: afeWakeup
//
// @brief: Wakes up AFE
void afeWakeup()
{
    afeStartComm();
    afeEndComm();
}

// @funcname: broadcastPoll
//
// @brief: Sends a broadcast message to AFE
//
// @param: command: 16 bit command to send to AFE
void broadcastPoll(uint16_t command)
{
    uint8_t message[4];
	uint16_t PEC;

	message[0] = command >> 8;
	message[1] = command;

	PEC = calculatePEC(message, 2);
	message[2] = PEC >> 8;
	message[3] = PEC;

	afeStartComm();
	HAL_SPI_Transmit(LTC6811_SPI, message, 4, HAL_MAX_DELAY);

	// Wait for LTC device to let go of SDO
	//while (HAL_GPIO_ReadPin(MISO_GPIO_Port, MISO_Pin) != GPIO_PIN_SET);

	afeEndComm();
}

// @funcname: broadcastWrite
//
// @brief: Sends a broadcast message to AFE with data to write
//
// @param: command: 16 bit command to send to AFE
// @param: size: data length to write
// @param: data: pointer to data to write
void broadcastWrite(uint16_t command, uint16_t size, uint8_t* data)
{
    uint16_t PEC;
	uint8_t message[4 + 6 + 2];

	message[0] = command >> 8;
	message[1] = command;
	PEC = calculatePEC(message, 2);
	message[2] = PEC >> 8;
	message[3] = PEC;

	memcpy(&message[4], data, 6);

	PEC = calculatePEC(data, size);
	message[4 + 6 + 0] = PEC >> 8;
	message[4 + 6 + 1] = PEC;

	afeStartComm();

	HAL_SPI_Transmit(LTC6811_SPI, message, 4 + size + 2, HAL_MAX_DELAY);

	afeEndComm();

	return;
}

// @funcname: broadcastRead
//
// @brief: Sends a broadcast message to AFE and reads data
//
// @param: command: 16 bit command to send to AFE
// @param: size: data length to write
// @param: data: pointer to data to write
int broadcastRead(uint16_t command, uint16_t size, uint8_t* data)
{

	uint8_t command_message[4];
	uint8_t data_PEC[2];
	uint16_t PEC;
	uint16_t command_PEC;

	command_message[0] = command >> 8;
	command_message[1] = command;
	command_PEC = calculatePEC(command_message, 2);
	command_message[2] = command_PEC >> 8;
	command_message[3] = command_PEC;

	afeStartComm();

	// Send command
	HAL_SPI_Transmit(LTC6811_SPI, command_message, 4, HAL_MAX_DELAY);

    // Receive data
	HAL_SPI_Receive(LTC6811_SPI, data, size, HAL_MAX_DELAY);

	// Receive PEC
	HAL_SPI_Receive(LTC6811_SPI, data_PEC, 2, HAL_MAX_DELAY);

	afeEndComm();

	// Verify PEC
	PEC = data_PEC[0] << 8 | data_PEC[1];
	if (verifyPEC(data, size, PEC) < 0)
    {
		return -1;
	}

	return 0;
}

// @funcname: afeStartComm
//
// @brief: Pulls CSB low to start comms
static void afeStartComm()
{
    HAL_GPIO_WritePin(CSB_AFE_GPIO_Port, CSB_AFE_Pin, GPIO_PIN_RESET);
}

// @funcname: afeEndComm
//
// @brief: Pulls CSB high to stop comms
static void afeEndComm()
{
    HAL_GPIO_WritePin(CSB_AFE_GPIO_Port, CSB_AFE_Pin, GPIO_PIN_SET);
}

// @funcname: calculatePEC
//
// @brief: Calculates 16 bit packet error code (PEC)
//
// @param: data: pointer to data
// @param: len: length of data
static uint16_t calculatePEC(uint8_t* data, uint8_t len)
{
    uint8_t i;
    uint16_t address;
    uint16_t remainder = 16;

	for (i = 0; i < len; i++)
    {
		address = ((remainder >> 7) ^ data[i]) & 0xff;
		remainder = (remainder << 8) ^ crc15Table[address];
	}

	return ((remainder * 2) & 0xffff);
}

// @funcname: verifyPEC
//
// @brief: Verifies a PEC
//
// @param: data: pointer to data to check
// @param: len: data length
// @param: PEC: PEC to check against
int verifyPEC(uint8_t* data, uint16_t len, uint16_t PEC)
{

	if (calculatePEC(data, len) == PEC)
    {
		return 0;
	}

	return -1;
}
