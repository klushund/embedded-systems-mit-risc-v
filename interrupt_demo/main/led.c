/*
 * Example of the book "Embedded Systems mit RISC-V", dpunkt.verlag
 * Author: Patrick Ritschel
 *
 * see https://ritschel.at/kapitel-6-interrupts-und-exceptions-beispiel-interrupt_demo/
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
#include "systick.h"
#include "led.h"

#define LED1_GPIO						CONFIG_LED1_GPIO

// ***** implementation *****

void led_init() {
	// initialize GPIOs, with implicit multiplexing
	gpio_config_t gpioConfig = {
		.pin_bit_mask = (1 << LED1_GPIO),
		.mode = GPIO_MODE_INPUT_OUTPUT,
		.pull_up_en = false,
		.pull_down_en = false,
		.intr_type = GPIO_INTR_DISABLE
	};
	gpio_config(&gpioConfig);
}

void led_switchLED(uint8_t key) {
	uint32_t ledLevel = gpio_get_level(LED1_GPIO);
	gpio_set_level(LED1_GPIO, !ledLevel);
}
