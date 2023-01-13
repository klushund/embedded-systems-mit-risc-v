/*
 * Example of the book "Embedded Systems mit RISC-V", dpunkt.verlag
 * Author: Patrick Ritschel
 *
 * see https://ritschel.at/kapitel-9-3-echtzeitbetriebssystem-beispiel-task_demo/
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
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "led_strip.h"

enum ButtonState {
	ButtonState_Pressed = 1,
	ButtonState_Released
};

struct ButtonEvent {
	uint64_t systemtime;
	enum ButtonState buttonState;
};

#define BTN_GPIO						GPIO_NUM_9

#define LED_TASK_STACKSIZE				2048
#define LED_TASK_PRIORITY				3
#define DEPOSIT_TASK_STACKSIZE			2048
#define DEPOSIT_TASK_PRIORITY			0
#define WITHDRAW_TASK_STACKSIZE			2048
#define WITHDRAW_TASK_PRIORITY			0
#define BUTTON_TASK_STACKSIZE			2048
#define BUTTON_TASK_PRIORITY			3

TaskHandle_t gLedTaskHandle = NULL;
TaskHandle_t gDepositTaskHandle = NULL;
TaskHandle_t gWithdrawTaskHandle = NULL;
TaskHandle_t gButtonTaskHandle = NULL;

static led_strip_handle_t gLedStrip;
static volatile int gSaldo = 0;

static SemaphoreHandle_t gSem = NULL;
static volatile uint32_t gRunning = 0;

static SemaphoreHandle_t gButtonSem = NULL;
#define QUEUE_LENGTH					10
#define QUEUE_ITEM_SIZE					sizeof(struct ButtonEvent)
static QueueHandle_t gQueue = NULL;
static StaticQueue_t gQueueMemory;
static uint8_t gQueueItemMemory[QUEUE_LENGTH * QUEUE_ITEM_SIZE];

static void initLED(void);
static void ledTaskMainFunc(void* pParameters);
static void depositTaskMainFunc(void* pParam);
static void withdrawTaskMainFunc(void* pParam);
static void buttonTaskMainFunc(void * pvParameters);

void initLED() {
    led_strip_config_t stripConfig = {
        .strip_gpio_num = 8,
        .max_leds = 1, // one LED on board
    };
    led_strip_rmt_config_t rmtConfig = {
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&stripConfig, &rmtConfig, &gLedStrip));
    led_strip_clear(gLedStrip);
}

void ledTaskMainFunc(void* pBlinkPeriod_ms) {
	bool lightUp = false;
	while (true) {
		led_strip_set_pixel(gLedStrip, 0, 0, 0, lightUp ? 10 : 0);
		led_strip_refresh(gLedStrip);
		#if CONFIG_PRINT_LEDTASK_STATUS == 1
		// get information about the task using vTaskGetInfo:
		TaskStatus_t tStat;
		vTaskGetInfo(gLedTaskHandle, &tStat, true, eInvalid);
		printf("%s: t=%ld, st=%ld\n", tStat.pcTaskName, tStat.ulRunTimeCounter, tStat.usStackHighWaterMark);
		#endif
		vTaskDelay(*(uint32_t*)pBlinkPeriod_ms / (portTICK_PERIOD_MS * 2));
		lightUp = !lightUp;
	}
}

void depositTaskMainFunc(void* pParam) {
	(void) pParam;
	printf("Deposit starting\n");
	int i = 1000000;
	gRunning |= 1;
	while (i > 0) {
		if (xSemaphoreTake(gSem, 100 / portTICK_PERIOD_MS) == true) {
//		if (xSemaphoreTake(gSem, portMAX_DELAY) == true) {
			gSaldo += 100;
			i = i - 1;
			xSemaphoreGive(gSem);
		}
	}
	printf("Deposit stopping\n");
	gRunning &= ~1;
	while (true) {
		// consume as much CPU time as possible
	}
}

void withdrawTaskMainFunc(void* pParam) {
	(void) pParam;
	printf("Withdraw starting\n");
	int i = 1000000;
	gRunning |= 2;
	while (i > 0) {
		if (xSemaphoreTake(gSem, 100 / portTICK_PERIOD_MS) == true) {
			gSaldo -= 100;
			i = i - 1;
			xSemaphoreGive(gSem);
		}
	}
	printf("Withdraw stopping\n");
	gRunning &= ~2;
	while (true) {
		// consume as much CPU time as possible
	}
}

void buttonTaskMainFunc(void * pvParameters) {
	struct ButtonEvent buttonEvent = {
			.buttonState = ButtonState_Released,
			.systemtime = 0
	};
	uint32_t btnActive = 0;

	while (true) {
		if (gpio_get_level(BTN_GPIO) == 0) { // pressed
			if (++btnActive == 2) { // pressed long enough
				buttonEvent.buttonState = ButtonState_Pressed;
				#if CONFIG_BUTTON_SYNCH_SEMAPHORE == 1
				if (xSemaphoreGive(gButtonSem) == false) {
				#elif CONFIG_BUTTON_SYNCH_QUEUE == 1
				if (xQueueSend(gQueue, &buttonEvent, 0) == false) {
				#endif
				printf("state send failed\n");
				}
			}
		} else {
			btnActive = 0;
			if (buttonEvent.buttonState == ButtonState_Pressed) {
				buttonEvent.buttonState = ButtonState_Released;
				#if CONFIG_BUTTON_SYNCH_SEMAPHORE == 1
				if (xSemaphoreGive(gButtonSem) == false) {
				#elif CONFIG_BUTTON_SYNCH_QUEUE == 1
				if (xQueueSend(gQueue, &buttonEvent, 0) == false) {
				#endif
					printf("state send failed\n");
				}
			}
		}
		vTaskDelay(50 / portTICK_PERIOD_MS);
	}
}

void app_main() {
	initLED();

	// blink task demo
	static uint32_t blinkPeriod_ms = 500;

	xTaskCreate(ledTaskMainFunc, "BLINK", LED_TASK_STACKSIZE, &blinkPeriod_ms, LED_TASK_PRIORITY, &gLedTaskHandle);
	configASSERT(gLedTaskHandle);

	// critical section demo, with deposit and withdraw
	vSemaphoreCreateBinary(gSem);
	configASSERT(gSem);

	xTaskCreate(depositTaskMainFunc, "DEPOSIT", DEPOSIT_TASK_STACKSIZE, NULL, DEPOSIT_TASK_PRIORITY, &gDepositTaskHandle);
	configASSERT(gDepositTaskHandle);
	xTaskCreate(withdrawTaskMainFunc, "WITHDRAW", WITHDRAW_TASK_STACKSIZE, NULL, WITHDRAW_TASK_PRIORITY, &gWithdrawTaskHandle);
	configASSERT(gWithdrawTaskHandle);

	// button task demo for producer/consumer
	gButtonSem = xSemaphoreCreateCounting(100, 0);
	configASSERT(gButtonSem);
	gQueue = xQueueCreateStatic(QUEUE_LENGTH, QUEUE_ITEM_SIZE, gQueueItemMemory, &gQueueMemory);

	xTaskCreate(buttonTaskMainFunc, "BUTTON", BUTTON_TASK_STACKSIZE, NULL, BUTTON_TASK_PRIORITY, &gButtonTaskHandle);
	assert(gButtonTaskHandle != NULL);

    while (true) {
		#if CONFIG_BUTTON_SYNCH_SEMAPHORE == 1
    	if (xSemaphoreTake(gButtonSem, 400 / portTICK_PERIOD_MS) == true) {
		#elif CONFIG_BUTTON_SYNCH_QUEUE == 1
    	struct ButtonEvent buttonEvent;
		if (xQueueReceive(gQueue, &buttonEvent, 400 / portTICK_PERIOD_MS) == true) {
			if (buttonEvent.buttonState == ButtonState_Pressed)
		#endif
			printf("Btn pressed.\n");
    	} else {
    		printf("main working, running %ld, saldo %d ...\n", gRunning, gSaldo);
    	}
    }
}
