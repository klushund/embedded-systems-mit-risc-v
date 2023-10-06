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
#include "button.h"

#define BTN_GPIO						CONFIG_BUTTON_GPIO

static void btn_isr_handler(void* arg);

#if CONFIG_CALLBACK_NONE == 1
bool gKeypressed = false;
#elif CONFIG_CALLBACK_STATIC == 1
void keyCallback(uint8_t key);
#elif CONFIG_CALLBACK_DYNAMIC == 1
static KeyCallback gKeyCallback = NULL;
#endif

// ***** implementation *****
void button_init() {
	gpio_config_t gpioConfigIn = {
		.pin_bit_mask = (1 << BTN_GPIO),
		.mode = GPIO_MODE_INPUT,
		.pull_up_en = true,
		.pull_down_en = false,
		.intr_type = GPIO_INTR_NEGEDGE
	};
	gpio_config(&gpioConfigIn);

	gpio_install_isr_service(0);
	gpio_isr_handler_add(BTN_GPIO, btn_isr_handler, NULL);
}

void btn_isr_handler(void* arg) {
	(void) arg;
	#if CONFIG_CALLBACK_NONE == 1
    gKeypressed = true;
	#elif CONFIG_CALLBACK_STATIC == 1
    keyCallback(BTN_GPIO);
	#elif CONFIG_CALLBACK_DYNAMIC == 1
    if (gKeyCallback != NULL) {
    	gKeyCallback(BTN_GPIO);
    }
	#endif
}

#if CONFIG_CALLBACK_STATIC == 1
__attribute__((weak)) void keyCallback(uint8_t key) {
	// default: ignore
}
#endif

#if CONFIG_CALLBACK_DYNAMIC == 1
void registerKeyCallback(KeyCallback keyCallback) {
	gKeyCallback = keyCallback;
}
#endif
