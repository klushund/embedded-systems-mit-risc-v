/*
 * Part of the book "Embedded Systems mit RISC-V", dpunkt.verlag
 * Author: Patrick Ritschel
 *
 * see https://ritschel.at/buch-embedded-systems-auf-den-punkt-gebracht/
 *
 * The code of this project is in the Public Domain (or CC0 licensed, at your option).
 * Unless required by applicable law or agreed to in writing, this
 * software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied.
 */

#include <stdlib.h>
#include "sdkconfig.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "pushbtn.h"

#define BUTTON_TASK_STACKSIZE		2048

// module internal globals
static const char* TAG = "pushbtn";
TaskHandle_t gButtonTaskHandle = NULL;
static MessageBufferHandle_t gObserverMessageBuffer = NULL;

static void buttonTaskMainFunc(void * pvParameters);

void pushbtn_init() {
	xTaskCreate(buttonTaskMainFunc, "BUTTON", BUTTON_TASK_STACKSIZE, NULL, 3, &gButtonTaskHandle);
	configASSERT(gButtonTaskHandle);
}

void pushbtn_registerObserver(MessageBufferHandle_t observerMB) {
	gObserverMessageBuffer = observerMB;
}

void buttonTaskMainFunc(void * pvParameters) {
	(void) pvParameters; // unused parameter

	struct ButtonEvent buttonEvent = {
			.buttonState = ButtonState_Released,
			.systemtime = 0
	};
	uint32_t btnActive = 0;

	while (true) {
		if (gpio_get_level(CONFIG_BUTTON_GPIO) == 0) { // pressed
			if (++btnActive == 2) { // pressed long enough
				buttonEvent.buttonState = ButtonState_Pressed;
				if (gObserverMessageBuffer != NULL) {
					if (xMessageBufferSend(gObserverMessageBuffer, (void *)&buttonEvent, sizeof(struct ButtonEvent), pdMS_TO_TICKS(100)) != sizeof(struct ButtonEvent)) {
						ESP_LOGI(TAG, "state send failed");
					}
				}
			}
		} else {
			btnActive = 0;
			if (buttonEvent.buttonState == ButtonState_Pressed) {
				buttonEvent.buttonState = ButtonState_Released;
				if (gObserverMessageBuffer != NULL) {
					if (xMessageBufferSend(gObserverMessageBuffer, (void *)&buttonEvent, sizeof(struct ButtonEvent), pdMS_TO_TICKS(100)) != sizeof(struct ButtonEvent)) {
						ESP_LOGI(TAG, "state send failed");
					}
				}
			}
		}
		vTaskDelay(50 / portTICK_PERIOD_MS);
	}
}
