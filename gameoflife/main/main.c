/*
 * Example of the book "Embedded Systems mit RISC-V", dpunkt.verlag
 * Author: Patrick Ritschel
 *
 * see https://ritschel.at/kapitel-8-5-1-timer-des-esp32-c3-applikation-gameoflife/
 *
 * The code of this project is in the Public Domain (or CC0 licensed, at your option).
 * Unless required by applicable law or agreed to in writing, this
 * software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied.
 */

#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gptimer.h"
#include "esp_log.h"
#include "esp_random.h"
#include "led_strip.h"

#define GAME_DIMENSION		5

static const char* TAG = "gameoflife";

static led_strip_handle_t gLedStrip;
static int8_t* gameField = NULL;
static bool refreshDisplay = true;

static void setGameFieldValue(int x, int y, int8_t value) {
	if (x < 0) { // wrap around
		x = GAME_DIMENSION - 1;
	}
	if (x >= GAME_DIMENSION) {
		x = 0;
	}
	if (y < 0) {
		y = GAME_DIMENSION - 1;
	}
	if (y >= GAME_DIMENSION) {
		y = 0;
	}
	gameField[x * GAME_DIMENSION + y] = value;
}

static int8_t getGameFieldValue(int x, int y) {
	if (x < 0) { // wrap around
		x = GAME_DIMENSION - 1;
	}
	if (x >= GAME_DIMENSION) {
		x = 0;
	}
	if (y < 0) {
		y = GAME_DIMENSION - 1;
	}
	if (y >= GAME_DIMENSION) {
		y = 0;
	}
	return gameField[x * GAME_DIMENSION + y];
}

static void generateGame() {
	#if (CONFIG_INITIALIZE_GAMEFIELD_RANDOM == 1)
	gameField = malloc(sizeof(int8_t) * GAME_DIMENSION * GAME_DIMENSION);
	assert (gameField != NULL);
	for (int i = 0; i < GAME_DIMENSION; i += 1) {
		for (int j = 0; j < GAME_DIMENSION; j += 1) {
			setGameFieldValue(i, j, esp_random() % 2);
		}
	}
	#else // (CONFIG_INITIALIZE_GAMEFIELD_RANDOM == 1)
	for (int i = 0; i < GAME_DIMENSION; i += 1) {
		for (int j = 0; j < GAME_DIMENSION; j += 1) {
			setGameFieldValue(i, j, 0);
		}
	}
	#if (CONFIG_INITIALIZE_GAMEFIELD_OSCILLATING == 1)
	// Oscillating object
	setGameFieldValue(1, 1, 1);
	setGameFieldValue(2, 1, 1);
	setGameFieldValue(3, 1, 1);
	#elif (CONFIG_INITIALIZE_GAMEFIELD_GLIDER == 1)
	// Glider
	setGameFieldValue(1, 1, 1);
	setGameFieldValue(2, 1, 1);
	setGameFieldValue(3, 1, 1);
	setGameFieldValue(3, 2, 1);
	setGameFieldValue(2, 3, 1);
	#endif
	#endif // (CONFIG_INITIALIZE_GAMEFIELD_RANDOM == 1)
}

uint32_t countNeighbors(int x, int y) {
	uint32_t count = 0;
	for (int i = -1; i <= 1; i += 1) {
		for (int j = -1; j <= 1; j += 1) {
			if ((i != 0) || (j != 0)) {
				if (getGameFieldValue(x + i, y + j) > 0) {
					count += 1;
				}
			}
		}
	}
	return count;
}

static void calculateGeneration() {
	int8_t* gameFieldCopy = calloc(GAME_DIMENSION * GAME_DIMENSION, sizeof(int8_t));
	assert(gameFieldCopy != NULL);
	for (int i = 0; i < GAME_DIMENSION; i += 1) {
		for (int j = 0; j < GAME_DIMENSION; j += 1) {
			uint32_t neighborCount = countNeighbors(i, j);
			if (getGameFieldValue(i, j) > 0) { // cell is alive
				if ((neighborCount < 2) || (neighborCount > 3)) {
					gameFieldCopy[i * GAME_DIMENSION + j] = -20; // cell dies
				} else {
					gameFieldCopy[i * GAME_DIMENSION + j] = getGameFieldValue(i, j);
				}
			} else { // cell is dead
				if (neighborCount == 3) {
					gameFieldCopy[i * GAME_DIMENSION + j] = 1; // cell is born
				} else {
					gameFieldCopy[i * GAME_DIMENSION + j] = getGameFieldValue(i, j);
				}
			}
		}
	}
	free(gameField);
	gameField = gameFieldCopy;
}

static bool displayGameState(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx) {
	// tell the compiler that the parameters are intentionally not used
	(void) timer;
	(void) edata;
	(void) user_ctx;

	static uint8_t stepsToGeneration = 50;
	for (int i = 0; i < GAME_DIMENSION; i += 1) {
		for (int j = 0; j < GAME_DIMENSION; j += 1) {
			int8_t val = getGameFieldValue(i, j);
			if (val >= 20) {
				// fix living -> green
				led_strip_set_pixel(gLedStrip, i * GAME_DIMENSION + j, 0, val, 0);
			} else if (val < 0) {
				// dying -> red
				led_strip_set_pixel(gLedStrip, i * GAME_DIMENSION + j, -val, 0, 0);
				setGameFieldValue(i, j, val + 1);
			} else if (val == 0) {
				// fix dead -> dark
				led_strip_set_pixel(gLedStrip, i * GAME_DIMENSION + j, 0, 0, 0);
			} else {
				// starting life -> blue
				led_strip_set_pixel(gLedStrip, i * GAME_DIMENSION + j, 0, 0, val);
				setGameFieldValue(i, j, val + 1);
			}
		}
	}
	refreshDisplay = true;
	if (stepsToGeneration-- == 0) {
		stepsToGeneration = 50;
		calculateGeneration();
	}
	return false; // no need to yield to other tasks immediately after the callback
}

void app_main(void) {
	ESP_LOGI(TAG, "Init LED");
    // Initialize WS2812 LED Strip with 25 pixels
    led_strip_config_t stripConfig = {
        .strip_gpio_num = 8,
        .max_leds = 25
    };
    led_strip_rmt_config_t rmtConfig = {
        .resolution_hz = 10 * 1000 * 1000 // 10MHz
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&stripConfig, &rmtConfig, &gLedStrip));
    led_strip_clear(gLedStrip);

	gptimer_handle_t timer = NULL;
	static gptimer_event_callbacks_t timerCallbacks = {
	    .on_alarm = displayGameState // register user callback
	};

	gptimer_config_t timerConfig = {
	    .clk_src = GPTIMER_CLK_SRC_DEFAULT,
	    .direction = GPTIMER_COUNT_UP,
	    .resolution_hz = 1 * 1000 * 1000, // 1 MHz, 1 tick = 1 us
	};
	ESP_ERROR_CHECK(gptimer_new_timer(&timerConfig, &timer));
	ESP_ERROR_CHECK(gptimer_register_event_callbacks(timer, &timerCallbacks, NULL));
	gptimer_alarm_config_t alarmConfig = {
	    .reload_count = 0, // counter will reload with 0 on alarm event
	    .alarm_count = 20000, // every 20 ms
	    .flags.auto_reload_on_alarm = true, // enable auto-reload
	};
	ESP_ERROR_CHECK(gptimer_set_alarm_action(timer, &alarmConfig));
	ESP_ERROR_CHECK(gptimer_enable(timer));

	generateGame();

	ESP_LOGI(TAG, "Game starting");
	ESP_ERROR_CHECK(gptimer_start(timer));

	while (1) {
        // update LEDs
		if (refreshDisplay) {
			led_strip_refresh(gLedStrip);
		}
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
