/*
 * Author: Patrick Ritschel
 *
 * see https://ritschel.at/bewegungsmelder/
 *
 * The code of this project is in the Public Domain (or CC0 licensed, at your option).
 * Unless required by applicable law or agreed to in writing, this
 * software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied.
 */

#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include "esp_check.h"
#include "pirsensor.h"

#define TAG		"pirsensor"

static void pirsensor_isr_handler(void* arg);


// ***** implementation *****
esp_err_t pirsensor_init(gpio_num_t gpio, PIRSensorHandle* pHandle) {
	PIRSensorHandle handle = malloc(sizeof(struct PIRSensorInfo));
	ESP_RETURN_ON_FALSE(handle, ESP_ERR_NO_MEM, TAG, "Out of memory");
	handle->gpio = gpio;
	handle->callback = NULL;
	*pHandle = handle;
	gpio_config_t gpioConfigIn = {
		.pin_bit_mask = (1 << gpio),
		.mode = GPIO_MODE_INPUT,
		.pull_up_en = false,
		.pull_down_en = true,
		.intr_type = GPIO_INTR_POSEDGE
	};
	ESP_RETURN_ON_ERROR(gpio_config(&gpioConfigIn), TAG, "gpio config failed");

	ESP_RETURN_ON_ERROR(gpio_install_isr_service(0), TAG, "isr service install failed");
	return gpio_isr_handler_add(gpio, pirsensor_isr_handler, handle);
}

void pirsensor_isr_handler(void* arg) {
	PIRSensorHandle handle = (PIRSensorHandle) arg;
    if (handle->callback != NULL) {
    	handle->callback(handle);
    }
}

void pirsensor_registerCallback(const PIRSensorHandle handle, PIRSensorCallback pirsensorCallback) {
	handle->callback = pirsensorCallback;
}
