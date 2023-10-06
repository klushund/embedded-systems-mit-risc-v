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
#include "max30102.h"

#define TAG		"MAX30102"
#define PROBLEM "I2C communication problem"

static esp_err_t softReset(void);
static esp_err_t setup(uint8_t mode, uint16_t samplingRate);
static uint16_t readFIFO(uint32_t* irValues, uint32_t* redValues, uint16_t maxcnt);

// ***** implementation *****
void max30102_initCallbacks(struct Max3010xDevice_t* pDevice) {
	pDevice->softResetFct = softReset;
	pDevice->setupFct = setup;
	pDevice->readFIFOFct = readFIFO;
}

esp_err_t softReset() {
	esp_err_t res = max3010x_readModifyWriteRegister(MAX30102_REG_MODECONFIG, ~MAX30102_MODECONFIG_RESET, MAX30102_MODECONFIG_RESET);
	ESP_RETURN_ON_ERROR(res, TAG, "Reset cannot be sent");
	// Poll for bit to clear, reset is then complete
	//todo implement a timeout
	uint8_t status;
	do {
		status = max3010x_readRegister(MAX30102_REG_MODECONFIG, &res);
		ESP_RETURN_ON_ERROR(res, TAG, PROBLEM);
		vTaskDelay(pdMS_TO_TICKS(1));
	} while (status & MAX30102_MODECONFIG_RESET);
	return ESP_OK;
}

// start with 100Hz sampling rate, 1600us pulse width, middle LED currents, high resolution (16 bit)
esp_err_t setup(uint8_t mode, uint16_t samplingRate) {
	// stop eventually ongoing sampling
	ESP_RETURN_ON_ERROR(softReset(), TAG, "Reset failed");

	// clear FIFO pointers
	ESP_RETURN_ON_ERROR(max3010x_writeRegister(MAX30102_REG_FIFOWRPTR, 0x00), TAG, PROBLEM);
	ESP_RETURN_ON_ERROR(max3010x_writeRegister(MAX30102_REG_FIFOOVFCTR, 0x00), TAG, PROBLEM);
	ESP_RETURN_ON_ERROR(max3010x_writeRegister(MAX30102_REG_FIFORDPTR, 0x00), TAG, PROBLEM);

	// initialize sampling
	// 800Hz, 8-times averaged -> 100 Hz samples
//	max3010x_readModifyWriteRegister(MAX30102_REG_FIFOCONFIG, 0x00, MAX30102_FIFOCONFIG_SMPAVE8);
//	max3010x_writeRegister(MAX30102_REG_SPO2CONFIG, MAX30102_SPO2CONFIG_SAMPLERATE_800 | MAX30102_SPO2CONFIG_LEDPW_215us_ADC17bit | MAX30102_SPO2CONFIG_ADCRGE_11);
	ESP_RETURN_ON_ERROR(max3010x_readModifyWriteRegister(MAX30102_REG_FIFOCONFIG, 0x00, MAX30102_FIFOCONFIG_SMPAVENONE | 0x0F), TAG, PROBLEM); // almost full: 17, no averaging
	ESP_RETURN_ON_ERROR(max3010x_writeRegister(MAX30102_REG_SPO2CONFIG, MAX30102_SPO2CONFIG_SAMPLERATE_100 | MAX30102_SPO2CONFIG_LEDPW_411us_ADC18bit | MAX30102_SPO2CONFIG_ADCRGE_01), TAG, PROBLEM);
	ESP_RETURN_ON_ERROR(max3010x_readModifyWriteRegister(MAX30102_REG_MODECONFIG, 0xF8, mode), TAG, PROBLEM);
	assert(samplingRate == 100);
	max3010x_getDevice()->redLEDCurrent = MAX30102_LEDCURRENT_uA_TO_REGVAL(7200); // 7,2 mA
	max3010x_getDevice()->irLEDCurrent = MAX30102_LEDCURRENT_uA_TO_REGVAL(7200); // 7,2 mA
	ESP_RETURN_ON_ERROR(max3010x_writeRegister(MAX30102_REG_LEDPULSEAMP1, max3010x_getDevice()->redLEDCurrent), TAG, PROBLEM);
	ESP_RETURN_ON_ERROR(max3010x_writeRegister(MAX30102_REG_LEDPULSEAMP2, max3010x_getDevice()->irLEDCurrent), TAG, PROBLEM);
	ESP_RETURN_ON_ERROR(max3010x_writeRegister(MAX30102_REG_PILOTPA, MAX30102_LEDCURRENT_uA_TO_REGVAL(25400)), TAG, PROBLEM); // 25,4 mA for Pilot LED
	// clear interrupt status
	esp_err_t res;
	uint8_t intStatus = max3010x_readRegister(MAX30102_REG_INTERRUPTSTATUS1, &res);
	ESP_RETURN_ON_ERROR(res, TAG, PROBLEM);
	printf("Setup intstatus1 %02X\n", intStatus);
	// initialize IRQ: interrupt when new data is available
	//max3010x_writeRegister(MAX30102_REG_INTERRUPTENABLE1, MAX30102_INT1_AFULL);
	//max3010x_writeRegister(MAX30102_REG_INTERRUPTENABLE2, 0x00);
	return ESP_OK;
}

uint16_t readFIFO(uint32_t* irValues, uint32_t* redValues, uint16_t maxcnt) {
//	// clear interrupt status
//	uint8_t intStatus = max3010x_readRegister(MAX30102_REG_INTERRUPTSTATUS1);
//	intStatus = max3010x_readRegister(MAX30102_REG_INTERRUPTSTATUS2);

	uint8_t reg = MAX30102_REG_FIFOWRPTR;
	uint8_t data[6 * MAX30102_FIFOSIZE];
	max3010x_writeReadDevice(&reg, 1, data, 3);
	uint16_t cnt = (data[0] - data[2]) & 0x1F;
	if (data[1]) {
		cnt = 32;
		printf("ALARM!!!! OVERFLOW!!!\n");
	}
	if (cnt > 0) {
		cnt = MIN(cnt, maxcnt);

		reg = MAX30102_REG_FIFODATA;
		max3010x_writeReadDevice(&reg, 1, data, 6 * cnt);
		for (uint8_t i = 0; i < cnt; i += 1) {
			uint32_t val = data[0 + i * 6];
			val <<= 8;
			val |= data[1 + i * 6];
			val <<= 8;
			val |= data[2 + i * 6];
			*redValues++ = val & 0x0003FFFF;
			val = data[3];
			val <<= 8;
			val |= data[4 + i * 6];
			val <<= 8;
			val |= data[5 + i * 6];
			*irValues++ = val & 0x0003FFFF;
		}
	}
	return cnt;
}
