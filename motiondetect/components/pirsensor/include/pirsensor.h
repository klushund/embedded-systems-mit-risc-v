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

#ifndef MAIN_PIRSENSOR_H_
#define MAIN_PIRSENSOR_H_

#include "esp_err.h"
#include "driver/gpio.h"

typedef struct PIRSensorInfo* PIRSensorHandle;

typedef void (*PIRSensorCallback)(const PIRSensorHandle handle);

struct PIRSensorInfo {
	gpio_num_t gpio;
	PIRSensorCallback callback;
};


esp_err_t pirsensor_init(gpio_num_t gpio, PIRSensorHandle* pHandle);

void pirsensor_registerCallback(const PIRSensorHandle handle, PIRSensorCallback pirsensorCallback);

#endif /* MAIN_PIRSENSOR_H_ */
