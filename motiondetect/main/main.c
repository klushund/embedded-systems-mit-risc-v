/*
 * Example of the book "Embedded Systems mit RISC-V", dpunkt.verlag
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
#include "esp_log.h"
#include "led_strip.h"
#include "pirsensor.h"

#define TAG						"motiondetect"

enum LEDMode {
	LEDMODE_DARK,
	LEDMODE_LITUP,
	LEDMODE_UNDEF
};

static PIRSensorHandle gPIRSensor = NULL;
static int gLEDActivationDownCount_s = CONFIG_PIR_LED_ACTIVATION_s;
static led_strip_handle_t gLedStrip;
static enum LEDMode gActiveLEDMode = LEDMODE_UNDEF;

static void initLED(void);
static void setLEDMode(enum LEDMode mode);
static void pirsensorCallback(const PIRSensorHandle handle);
void app_main(void);

// ***** implementation *****
void initLED() {
    led_strip_config_t stripConfig = {
        .strip_gpio_num = CONFIG_LEDSTRIP_GPIO,
        .max_leds = 1, // one LED on board
    };
    led_strip_rmt_config_t rmtConfig = {
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&stripConfig, &rmtConfig, &gLedStrip));
    led_strip_clear(gLedStrip);
    setLEDMode(LEDMODE_LITUP);
}

void setLEDMode(enum LEDMode mode) {
	if (mode != gActiveLEDMode) {
		gActiveLEDMode = mode;
		switch (mode) {
			case LEDMODE_LITUP:
				led_strip_set_pixel(gLedStrip, 0, CONFIG_LED_LITUP_VALUE, CONFIG_LED_LITUP_VALUE, 0);
				break;
			default:
				led_strip_set_pixel(gLedStrip, 0, 0, 0, CONFIG_LED_DARK_VALUE);
				break;
		}
		led_strip_refresh(gLedStrip);
	}
}

void pirsensorCallback(const PIRSensorHandle handle) {
	(void) handle;
	gLEDActivationDownCount_s = CONFIG_PIR_LED_ACTIVATION_s;
}

void app_main(void) {
	ESP_LOGI(TAG, "Startup " TAG);

	initLED();

	pirsensor_init(CONFIG_PIRSENSOR_GPIO, &gPIRSensor);
	pirsensor_registerCallback(gPIRSensor, pirsensorCallback);

    while (true) {
        if (gLEDActivationDownCount_s > 0) {
        	if (--gLEDActivationDownCount_s <= 0) {
        		setLEDMode(LEDMODE_DARK);
        	} else {
        		setLEDMode(LEDMODE_LITUP);
        	}
        }
        sleep(1);
    }
}
