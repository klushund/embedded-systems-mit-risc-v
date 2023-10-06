/*
 * Example of the book "Embedded Systems mit RISC-V", dpunkt.verlag
 * Author: Patrick Ritschel
 *
 * see https://ritschel.at/kapitel-5-5-taster-anschliessen-beispiel-leds_and_button/
 *
 * Based on the basic Espressif IDF-project.
 *
 * The code of this project is in the Public Domain (or CC0 licensed, at your option).
 * Unless required by applicable law or agreed to in writing, this
 * software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied.
 */
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define LED1_GPIO						CONFIG_LED1_GPIO
#define LED2_GPIO						CONFIG_LED2_GPIO
#define BTN_GPIO						CONFIG_BUTTON_GPIO

// ***** implementation *****
void app_main() {
	printf("Two LEDs and a button via ESP-IDF demo\n");
	// initialize GPIOs, with implicit multiplexing
	gpio_config_t gpioConfig = {
		.pin_bit_mask = (1 << LED1_GPIO) | (1 << LED2_GPIO),
		.mode = GPIO_MODE_OUTPUT,
		.pull_up_en = false,
		.pull_down_en = false,
		.intr_type = GPIO_INTR_DISABLE
	};
	gpio_config(&gpioConfig);
	gpio_config_t gpioConfigIn = {
		.pin_bit_mask = (1 << BTN_GPIO),
		.mode = GPIO_MODE_INPUT,
		.pull_up_en = true,
		.pull_down_en = false,
		.intr_type = GPIO_INTR_DISABLE
	};
	gpio_config(&gpioConfigIn);

	uint32_t ledLevel = 0;
	uint32_t btnActive = 0;
	while (true) {
		if (gpio_get_level(BTN_GPIO) == 0) { // pressed
			if (++btnActive == 2) { // pressed long enough
				ledLevel = !ledLevel;
				gpio_set_level(LED1_GPIO, ledLevel);
				gpio_set_level(LED2_GPIO, ledLevel);
			}
		} else {
			btnActive = 0;
		}
		vTaskDelay(50 / portTICK_PERIOD_MS);
	}
}
