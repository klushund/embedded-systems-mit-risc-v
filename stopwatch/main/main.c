/*
 * Example of the book "Embedded Systems mit RISC-V", dpunkt.verlag
 * Author: Patrick Ritschel
 *
 * see https://ritschel.at/stoppuhr-uebungsbeispiel/
 *
 * The code of this project is in the Public Domain (or CC0 licensed, at your option).
 * Unless required by applicable law or agreed to in writing, this
 * software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied.
 */

#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "driver/gptimer.h"
#include "esp_log.h"
#include "led_strip.h"
#include "graphics.h"
#include "pushbtn.h"

#define DISPLAY_INTERFACE				I2C_NUM_0
#define DISPLAY_I2C_ADDR				0x3C

static const char* TAG = "stopwatch";

static led_strip_handle_t gLedStrip;
static gptimer_handle_t gTimer = NULL;
static bool gStopwatchRunning = false;

static void initI2C(i2c_port_t i2c_num);
static void initLed(void);

void initStopwatch(void);
static void startStopwatch(void);
static void stopStopwatch(void);
uint32_t getStopwatchCounter(void);

// ***** Implementation *****
void initI2C(i2c_port_t i2c_num) {
	ESP_LOGI(TAG, "init I2C");
	i2c_config_t conf = {
		.mode = I2C_MODE_MASTER,
		.sda_io_num = 5,
		.scl_io_num = 6,
		.sda_pullup_en = GPIO_PULLUP_ENABLE,
		.scl_pullup_en = GPIO_PULLUP_ENABLE,
		.master.clk_speed = 400000
	};
	i2c_param_config(i2c_num, &conf);
	ESP_ERROR_CHECK(i2c_driver_install(i2c_num, conf.mode, 0, 0, 0));
}

void initLed() {
    ESP_LOGI(TAG, "Example configured to blink addressable LED!");
    led_strip_config_t stripConfig = {
        .strip_gpio_num = 2,
        .max_leds = 1
    };
    led_strip_rmt_config_t rmtConfig = {
        .resolution_hz = 10 * 1000 * 1000 // 10MHz
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&stripConfig, &rmtConfig, &gLedStrip));
    led_strip_clear(gLedStrip);
}

void initStopwatch() {
	gptimer_config_t timerConfig = {
	    .clk_src = GPTIMER_CLK_SRC_DEFAULT,
	    .direction = GPTIMER_COUNT_UP,
	    .resolution_hz = 1 * 1000 * 1000, // 1MHz, 1 tick = 1us
	};
	ESP_ERROR_CHECK(gptimer_new_timer(&timerConfig, &gTimer));
	gptimer_enable(gTimer);
}

void startStopwatch() {
	gptimer_set_raw_count(gTimer, 0);
	gptimer_start(gTimer);
	led_strip_set_pixel(gLedStrip, 0, 0, 10, 0);
	led_strip_refresh(gLedStrip);
	gStopwatchRunning = true;
}

void stopStopwatch() {
	gptimer_stop(gTimer);
	led_strip_set_pixel(gLedStrip, 0, 10, 0, 0);
	led_strip_refresh(gLedStrip);
	gStopwatchRunning = false;
}

uint32_t getStopwatchCounter() { // ms resolution
	uint64_t timerValue;
	gptimer_get_raw_count(gTimer, &timerValue);
	return (uint32_t)(timerValue / 1000);
}

void app_main() {
	initLed();

	initI2C(DISPLAY_INTERFACE);
	graphics_init(DISPLAY_INTERFACE, CONFIG_GRAPHICS_PIXELWIDTH + 28, CONFIG_GRAPHICS_PIXELHEIGHT, 28, false);

	pushbtn_init();

	MessageBufferHandle_t buttonMessageBuffer = xMessageBufferCreate(sizeof(struct ButtonEvent) * 8);
	configASSERT(buttonMessageBuffer);
	pushbtn_registerObserver(buttonMessageBuffer);

	initStopwatch();

	stopStopwatch();

	char buf[32];
    while (true) {

    	graphics_startUpdate();
	    graphics_clearScreen();

		uint32_t counter = getStopwatchCounter();
		uint32_t counter_s = counter / 1000;
		uint32_t counter_ms = counter % 1000;
	    if (gStopwatchRunning) {
	    	graphics_println("RUN");
	    	sprintf(buf, "%ld.%ld", counter_s, (counter_ms / 10));
	    	graphics_println(buf);
	    } else {
	    	graphics_println("STOP");
	    	sprintf(buf, "%ld.%03ld s", counter_s, counter_ms);
	    	graphics_println(buf);
	    }
		graphics_finishUpdate();

		struct ButtonEvent buttonEvent;
		if (xMessageBufferReceive(buttonMessageBuffer, &buttonEvent, sizeof(struct ButtonEvent), pdMS_TO_TICKS(10)) == sizeof(struct ButtonEvent)) {
			if (buttonEvent.buttonState == ButtonState_Pressed) {
				if (gStopwatchRunning) {
					stopStopwatch();
				} else {
					startStopwatch();
				}
			}
		}
    }
}
