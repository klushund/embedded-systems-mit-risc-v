/*
 * Example of the book "Embedded Systems mit RISC-V", dpunkt.verlag
 * Author: Patrick Ritschel
 *
 * see https://ritschel.at/kapitel-6-interrupts-und-exceptions-beispiel-interrupt_demo/
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
#include "button.h"
#include "led.h"
#include "systick.h"

#if CONFIG_CALLBACK_STATIC == 1
void keyCallback(uint8_t key);
#endif

// ***** implementation *****
#if (CONFIG_CALLBACK_STATIC == 1) || (CONFIG_CALLBACK_DYNAMIC == 1)
void keyCallback(uint8_t key) {
	led_switchLED(key);
}
#endif

void app_main() {
	printf("interrupt_demo\n");

	systick_init();
	led_init();
	button_init();

	#if CONFIG_CALLBACK_DYNAMIC == 1
	registerKeyCallback(keyCallback);
	#endif

    while (true) {
		#if CONFIG_CALLBACK_NONE == 1
    	if (gKeypressed) {
			led_switchLED(0);
    		gKeypressed = false;
    	}
		#endif
    	printf("Systick: %lld ms\n", getSysticks());
    	vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}
