/*
 * Example of the book "Embedded Systems mit RISC-V", dpunkt.verlag
 * Author: Patrick Ritschel
 *
 * see https://ritschel.at/buch-embedded-systems-auf-den-punkt-gebracht/
 *
 * The code of this project is in the Public Domain (or CC0 licensed, at your option).
 * Unless required by applicable law or agreed to in writing, this
 * software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied.
 */

#ifndef COMPONENTS_PULSEOXI_INCLUDE_MAX3010X_H_
#define COMPONENTS_PULSEOXI_INCLUDE_MAX3010X_H_

#include <stdint.h>
#include "driver/i2c.h"

#define MAX3010X_TAG									"max3010x"
#define MAX3010X_I2C_ADDR								0x57
#define MAX3010X_I2C_TIMEOUT_ms							1000

#define MAX3010X_PARTID_MAX30100						0x11
#define MAX3010X_PARTID_MAX30102						0x15

// registers
#define MAX3010X_REG_REVISION							0xFE
#define MAX3010X_REG_PARTID								0xFF

// masks
#define MAX3010X_MODE_HR_ONLY							0x02
#define MAX3010X_MODE_SPO2_HR							0x03

// function pointers
typedef esp_err_t (*SoftResetFct_t)(void);
typedef esp_err_t (*SetupFct_t)(uint8_t mode, uint16_t samplingRate);
typedef uint16_t (*ReadFIFOFct_t)(uint32_t* irValues, uint32_t* redValues, uint16_t maxcnt);
typedef void (*Max3010x_DataAvailableCallback_t)();

// device struct
struct Max3010xDevice_t {
	i2c_port_t i2cPort;
	uint8_t partID;
	uint8_t revision;

	// for setting the LEDs currents
	uint8_t irLEDCurrent;
	uint8_t redLEDCurrent;

	// function pointers
	SoftResetFct_t softResetFct;
	SetupFct_t setupFct;
	ReadFIFOFct_t readFIFOFct;
};

// function prototypes
// the given i2c port needs to be initialized previously, max. data rate = 400kHz
esp_err_t max3010x_init(i2c_port_t i2cPort, gpio_num_t gpioIRQ, Max3010x_DataAvailableCallback_t dataAvailableCB);
struct Max3010xDevice_t* max3010x_getDevice();
const char* max3010x_getDeviceName(const struct Max3010xDevice_t* pDevice);

esp_err_t max3010x_writeReadDevice(const uint8_t* write_buffer, size_t write_size, uint8_t* read_buffer, size_t read_size);
uint8_t max3010x_readRegister(uint8_t reg, esp_err_t* pRes);
esp_err_t max3010x_writeRegister(uint8_t reg, uint8_t value);
esp_err_t max3010x_readModifyWriteRegister(uint8_t reg, uint8_t mask, uint8_t value);

esp_err_t max3010x_softReset(void);
esp_err_t max3010x_setup(uint8_t mode, uint16_t samplingRate);
uint16_t max3010x_readFIFO(uint32_t* irValues, uint32_t* redValues, uint16_t maxcnt);

#endif /* COMPONENTS_PULSEOXI_INCLUDE_MAX3010X_H_ */
