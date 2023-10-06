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

#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_check.h"

#include "max30100.h"
#include "max30102.h"
#include "max3010x.h"

#define TAG			"MAX3010x"
#define PROBLEM 	"I2C communication problem"

static struct Max3010xDevice_t gDevice = { 0 };
static Max3010x_DataAvailableCallback_t gDataAvailableCB;

static void IRAM_ATTR gpioISR(void* arg);


// Note: callback will be called from interrupt context!
// if no IRQ line is connected, use GPIO_NUM_NC as gpioIRQ
esp_err_t max3010x_init(i2c_port_t i2cPort, gpio_num_t gpioIRQ, Max3010x_DataAvailableCallback_t dataAvailableCB) {
	gDevice.i2cPort = i2cPort;
	gDataAvailableCB = dataAvailableCB;

	// read out part ID
	esp_err_t res = ESP_OK;
	gDevice.partID = max3010x_readRegister(MAX3010X_REG_PARTID, &res);
	ESP_RETURN_ON_ERROR(res, TAG, PROBLEM);
	gDevice.revision = max3010x_readRegister(MAX3010X_REG_REVISION, &res);
	ESP_RETURN_ON_ERROR(res, TAG, PROBLEM);
	switch (gDevice.partID) {
		case MAX3010X_PARTID_MAX30100:
			ESP_LOGI(MAX3010X_TAG, "init max3010X, found MAX30100 rev %d", gDevice.revision);
			max30100_initCallbacks(&gDevice);
			break;
		case MAX3010X_PARTID_MAX30102:
			ESP_LOGI(MAX3010X_TAG, "init max3010X, found MAX30102 rev %d", gDevice.revision);
			max30102_initCallbacks(&gDevice);
			break;
		default:
			assert(false);
	}
	// setup IRQ line
	if (gpioIRQ != GPIO_NUM_NC) {
		gpio_config_t gpioIRQConfig = {
				.pin_bit_mask = (1 << gpioIRQ),
				.mode = GPIO_MODE_DEF_INPUT,
				.pull_up_en = GPIO_PULLUP_ENABLE,
				.pull_down_en = GPIO_PULLDOWN_DISABLE,
				.intr_type = GPIO_INTR_NEGEDGE
		};
		ESP_RETURN_ON_ERROR(gpio_config(&gpioIRQConfig), TAG, "Problem during gpio_config");
		//hook isr handler for specific gpio pin
		ESP_RETURN_ON_ERROR(gpio_isr_handler_add(gpioIRQ, gpioISR, NULL), TAG, "Problem during gpio_isr_handler_add");
	}
	return ESP_OK;
}

struct Max3010xDevice_t* max3010x_getDevice() {
	return &gDevice;
}

const char* max3010x_getDeviceName(const struct Max3010xDevice_t* pDevice) {
	if (pDevice != NULL) {
		switch (pDevice->partID) {
			case MAX3010X_PARTID_MAX30100:
				return "MAX30100";
			case MAX3010X_PARTID_MAX30102:
				return "MAX30102";
			default:
				return "Unknown";
		}
	}
	return NULL;
}

esp_err_t max3010x_writeReadDevice(const uint8_t* write_buffer, size_t write_size, uint8_t* read_buffer, size_t read_size) {
	return i2c_master_write_read_device(gDevice.i2cPort, MAX3010X_I2C_ADDR, write_buffer, write_size, read_buffer, read_size, pdMS_TO_TICKS(MAX3010X_I2C_TIMEOUT_ms));
}

uint8_t max3010x_readRegister(uint8_t reg, esp_err_t* pRes) {
	uint8_t data;
	*pRes = max3010x_writeReadDevice(&reg, 1, &data, 1);
	return data;
}

esp_err_t max3010x_writeRegister(uint8_t reg, uint8_t value) {
	uint8_t buf[2] = { reg, 0 };
	buf[1] = value;
	return i2c_master_write_to_device(gDevice.i2cPort, MAX3010X_I2C_ADDR, buf, 2, pdMS_TO_TICKS(MAX3010X_I2C_TIMEOUT_ms));
}

//Given a register, read it, mask it, and then set the thing
esp_err_t max3010x_readModifyWriteRegister(uint8_t reg, uint8_t mask, uint8_t value) {
	// Grab current register context
	uint8_t buf[2] = { reg, 0 };
	max3010x_writeReadDevice(buf, 1, (buf + 1), 1);
	// Zero-out the portions of the register we're interested in
	buf[1] &= mask;
	buf[1] |= value;
	// Change contents
	return i2c_master_write_to_device(gDevice.i2cPort, MAX3010X_I2C_ADDR, buf, 2, pdMS_TO_TICKS(MAX3010X_I2C_TIMEOUT_ms));
}

esp_err_t max3010x_softReset() {
	assert(gDevice.softResetFct != NULL);
	return gDevice.softResetFct();
}

esp_err_t max3010x_setup(uint8_t mode, uint16_t samplingRate) {
	assert(gDevice.setupFct != NULL);
	return gDevice.setupFct(mode, samplingRate);
}

uint16_t max3010x_readFIFO(uint32_t* irValue, uint32_t* redValue, uint16_t maxcnt) {
	assert(gDevice.readFIFOFct != NULL);
	return gDevice.readFIFOFct(irValue, redValue, maxcnt);
}

void gpioISR(void* arg) {
	if (gDataAvailableCB != NULL) {
		gDataAvailableCB();
	}
}
