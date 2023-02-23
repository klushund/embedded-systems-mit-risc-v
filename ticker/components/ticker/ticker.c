/*
 * Example of the book "Embedded Systems mit RISC-V", dpunkt.verlag
 * Author: Patrick Ritschel
 *
 * see https://ritschel.at/laufschrift-uebungsbeispiel/
 *
 * The code of this project is in the Public Domain (or CC0 licensed, at your option).
 * Unless required by applicable law or agreed to in writing, this
 * software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied.
 */
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "led_strip.h"
#include "rmt_led_patch.h"
#include "ticker.h"

#include "fontimgs_5x5.inc"

#define TAG "tickerapp"
#define TICKER_TASK_STACKSIZE		2048
#define TICKER_MAXTEXTLEN			127

enum Color {
	Color_Dark,
	Color_Red,
	Color_Green,
	Color_Blue,
	Color_Orange,
	Color_White
};

static led_strip_handle_t gLedStrip;
static char gTickerText[TICKER_MAXTEXTLEN+1] = { 0 };
static bool gRestartTicker = true;
static uint16_t gDisplayBrightness = CONFIG_DISPLAY_BRIGHTNESS;
static TickerCallback gTickerCallback = NULL;

// gPixels contains the display's pixels line by line
static uint8_t gPixels[CONFIG_DISPLAY_WIDTH * CONFIG_DISPLAY_HEIGHT] = { 0 };

TaskHandle_t gTickerTaskHandle = NULL;

static void getNextCharacterLine(uint8_t* line);
static void tickerTaskMainFunc(void * pvParameters);

typedef struct {
    led_strip_t base;
    rmt_channel_handle_t rmt_chan;
    rmt_encoder_handle_t strip_encoder;
    uint32_t strip_len;
    uint8_t bytes_per_pixel;
    uint8_t pixel_buf[];
} led_strip_rmt_obj;

// ***** implementation *****
esp_err_t ticker_init(gpio_num_t gpio) {
	ESP_LOGI(TAG, "Initialize ticker module");
    led_strip_config_t stripConfig = {
        .strip_gpio_num = gpio,
        .max_leds = CONFIG_DISPLAY_WIDTH * CONFIG_DISPLAY_HEIGHT
    };
    led_strip_rmt_config_t rmtConfig = {
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
    };
    esp_err_t status = led_strip_new_rmt_device(&stripConfig, &rmtConfig, &gLedStrip);
    if (status == ESP_OK) {
    	rmt_led_patch_changeChannel(&stripConfig, &rmtConfig, gLedStrip);
    	led_strip_clear(gLedStrip);
    }
    xTaskCreate(tickerTaskMainFunc, "ticker", TICKER_TASK_STACKSIZE, NULL, 3, &gTickerTaskHandle);
    return status;
}

void ticker_registerTickerCallback(TickerCallback tickerCallback) {
	gTickerCallback = tickerCallback;
}

// set text to NULL to clear
void ticker_setText(char* tickerText, bool restartTicker) {
	strncpy(gTickerText, tickerText, TICKER_MAXTEXTLEN);
	gRestartTicker = restartTicker;
}

void ticker_setDisplayBrightness(uint8_t brightness) {
	gDisplayBrightness = brightness;
}

uint8_t ticker_getDefaultDisplayBrightness() {
	return CONFIG_DISPLAY_BRIGHTNESS;
}

void getNextCharacterLine(uint8_t* line) {
	static uint32_t currentTickerTextOffs = 0;
	static uint16_t currentCharacterLine = 0;
	static char color = '\1';

	if ((currentTickerTextOffs >= strlen(gTickerText)) || (currentTickerTextOffs >= TICKER_MAXTEXTLEN)) {
		gRestartTicker = true;
	}
	if (gRestartTicker) {
		if (gTickerCallback != NULL) {
			gTickerCallback(TickerEvent_RestartTicker);
		}
		currentTickerTextOffs = 0;
		currentCharacterLine = 0;
		gRestartTicker = false;
	}
	if (gTickerText[currentTickerTextOffs] == '\0') {
		memset(line, 0x00, CONFIG_DISPLAY_HEIGHT);
		currentTickerTextOffs = 0;
	} else if (gTickerText[currentTickerTextOffs] == '\1') {
		currentTickerTextOffs += 1;
		switch(gTickerText[currentTickerTextOffs++]) {
			case 'r': color = Color_Red; break;
			case 'g': color = Color_Green; break;
			case 'b': color = Color_Blue; break;
			case 'o': color = Color_Orange; break;
			default: color = Color_White; break;
		}
	} else {
		uint16_t currentCharIdx = (uint16_t)gTickerText[currentTickerTextOffs];
		uint8_t charLine = 0x00;
		if (currentCharacterLine < gFont_fontimgs_5x5_CharacterOffsets[currentCharIdx+1] - gFont_fontimgs_5x5_CharacterOffsets[currentCharIdx]) {
			charLine = gFont_fontimgs_5x5_Characters[gFont_fontimgs_5x5_CharacterOffsets[currentCharIdx] + currentCharacterLine];
		}
		for (int i = 0; i < CONFIG_DISPLAY_HEIGHT; i += 1) {
			if (charLine & (0x01 << i)) {
				line[i] = color;
			} else {
				line[i] = Color_Dark;
			}
		}
		currentCharacterLine += 1;
		if (currentCharacterLine >= (gFont_fontimgs_5x5_CharacterOffsets[currentCharIdx+1] - gFont_fontimgs_5x5_CharacterOffsets[currentCharIdx]) + CONFIG_DISPLAY_INTERCHARACTERSPACE) {
			currentTickerTextOffs += 1;
			currentCharacterLine = 0;
			if (gTickerText[currentTickerTextOffs] == '\0') {
				gRestartTicker = true;
			}
		}
	}
}

void tickerTaskMainFunc(void * pvParameters) {
	(void) pvParameters; // unused
	while (true) {
		// shift one line to the left
		memmove(gPixels + CONFIG_DISPLAY_HEIGHT, gPixels, CONFIG_DISPLAY_WIDTH * CONFIG_DISPLAY_HEIGHT - CONFIG_DISPLAY_HEIGHT);
		getNextCharacterLine(gPixels);

		// copy the display buffer to the LEDs
		for (int i = 0; i < CONFIG_DISPLAY_WIDTH * CONFIG_DISPLAY_HEIGHT; i += 1) {
			switch (gPixels[i]) {
				case Color_Red:
					led_strip_set_pixel(gLedStrip, i, gDisplayBrightness, 0, 0);
					break;
				case Color_Green:
					led_strip_set_pixel(gLedStrip, i, 0, gDisplayBrightness, 0);
					break;
				case Color_Blue:
					led_strip_set_pixel(gLedStrip, i, 0, 0, gDisplayBrightness);
					break;
				case Color_Orange:
					led_strip_set_pixel(gLedStrip, i, gDisplayBrightness, (gDisplayBrightness * 2) / 3, 0);
					break;
				case Color_White:
					led_strip_set_pixel(gLedStrip, i, (gDisplayBrightness * 1) / 3, (gDisplayBrightness * 1) / 3, (gDisplayBrightness * 1) / 3);
					break;
				default:
					led_strip_set_pixel(gLedStrip, i, 0, 0, 0);
			}
		}
		led_strip_refresh(gLedStrip);
		usleep(CONFIG_SCROLL_DELAY * 1000);
	}
}
