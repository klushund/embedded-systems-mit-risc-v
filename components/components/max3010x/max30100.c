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

#include "esp_check.h"
#include "max30100.h"

#define TAG		"MAX30100"
#define PROBLEM "I2C communication problem"

static esp_err_t softReset(void);
static esp_err_t setup(uint8_t mode, uint16_t samplingRate);
static uint16_t readFIFO(uint32_t* irValues, uint32_t* redValues, uint16_t maxcnt);


void max30100_initCallbacks(struct Max3010xDevice_t* pDevice) {
	pDevice->softResetFct = softReset;
	pDevice->setupFct = setup;
	pDevice->readFIFOFct = readFIFO;
}

esp_err_t softReset() {
	esp_err_t res = max3010x_readModifyWriteRegister(MAX30100_REG_MODECONFIG, ~MAX30100_MODECONFIG_RESET, MAX30100_MODECONFIG_RESET);
	ESP_RETURN_ON_ERROR(res, TAG, "Reset cannot be sent");
	// Poll for bit to clear, reset is then complete
	//todo implement a timeout
	uint8_t status;
	do {
		status = max3010x_readRegister(MAX30100_REG_MODECONFIG, &res);
		ESP_RETURN_ON_ERROR(res, TAG, PROBLEM);
		vTaskDelay(pdMS_TO_TICKS(1));
	} while (status & MAX30100_MODECONFIG_RESET);
	return ESP_OK;
}

// start with 100Hz sampling rate, 1600us pulse width, middle LED currents, high resolution (16 bit)
esp_err_t setup(uint8_t mode, uint16_t samplingRate) {
	// stop eventually ongoing sampling
	ESP_RETURN_ON_ERROR(softReset(), TAG, PROBLEM);

	// clear FIFO pointers
	ESP_RETURN_ON_ERROR(max3010x_writeRegister(MAX30100_REG_FIFOWRPTR, 0x00), TAG, PROBLEM);
	ESP_RETURN_ON_ERROR(max3010x_writeRegister(MAX30100_REG_FIFOOVFCTR, 0x00), TAG, PROBLEM);
	ESP_RETURN_ON_ERROR(max3010x_writeRegister(MAX30100_REG_FIFORDPTR, 0x00), TAG, PROBLEM);

	// initialize sampling
	ESP_RETURN_ON_ERROR(max3010x_readModifyWriteRegister(MAX30100_REG_MODECONFIG, 0xF8, mode), TAG, PROBLEM);
	assert(samplingRate == 100);
	ESP_RETURN_ON_ERROR(max3010x_readModifyWriteRegister(MAX30100_REG_SPO2CONFIG, 0xE3, MAX30100_SAMPLINGRATE_100Hz << 2), TAG, PROBLEM);
	ESP_RETURN_ON_ERROR(max3010x_readModifyWriteRegister(MAX30100_REG_SPO2CONFIG, 0xFC, MAX30100_LEDPULSEWIDTH_1600us_ADC16), TAG, PROBLEM);
	max3010x_getDevice()->redLEDCurrent = MAX30100_LEDCURRENT_27_1mA;
	max3010x_getDevice()->irLEDCurrent = MAX30100_LEDCURRENT_27_1mA;
	ESP_RETURN_ON_ERROR(max3010x_writeRegister(MAX30100_REG_LEDCONFIG, (max3010x_getDevice()->redLEDCurrent << 4) | max3010x_getDevice()->irLEDCurrent), TAG, PROBLEM);
	ESP_RETURN_ON_ERROR(max3010x_readModifyWriteRegister(MAX30100_REG_SPO2CONFIG, ~MAX30100_SPO2CONFIG_HIRESEN, MAX30100_SPO2CONFIG_HIRESEN), TAG, PROBLEM);
	return ESP_OK;
}

uint16_t readFIFO(uint32_t* irValues, uint32_t* redValues, uint16_t maxcnt) {
	uint8_t reg = MAX30100_REG_FIFOWRPTR;
	uint8_t data[4 * 16];
	max3010x_writeReadDevice(&reg, 1, data, 3);
	uint16_t cnt = (data[0] - data[2]) & 0xF;
	if (data[1]) {
		cnt = 16;
		printf("ALARM!!!! OVERFLOW!!!\n");
	}
	if (cnt > 0) {
		cnt = MIN(cnt, maxcnt);

		reg = MAX30100_REG_FIFODATA;
		max3010x_writeReadDevice(&reg, 1, data, 4 * cnt);
		for (uint8_t i = 0; i < cnt; i += 1) {
			*irValues++ = (data[0] << 8) | data[1];
			*redValues++ = (data[2] << 8) | data[3];
		}
	}
	return cnt;
}
