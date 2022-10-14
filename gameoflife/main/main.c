#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/timer.h"
#include "led_strip.h"

#define GAME_DIMENSION		5

static led_strip_t *gpLEDStrip;
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
	gameField = malloc(sizeof(int8_t) * GAME_DIMENSION * GAME_DIMENSION);
	assert (gameField != NULL);
	for (int i = 0; i < GAME_DIMENSION; i += 1) {
		for (int j = 0; j < GAME_DIMENSION; j += 1) {
			//setGameFieldValue(i, j, rand() % 2);
			setGameFieldValue(i, j, esp_random() % 2);
		}
	}
//	for (int i = 0; i < GAME_DIMENSION; i += 1) {
//		for (int j = 0; j < GAME_DIMENSION; j += 1) {
//			setGameFieldValue(i, j, 0);
//		}
//	}
//	// Oszilating object
//	setGameFieldValue(1, 1, 1);
//	setGameFieldValue(2, 1, 1);
//	setGameFieldValue(3, 1, 1);
//	// Glider
//	setGameFieldValue(1, 1, 1);
//	setGameFieldValue(2, 1, 1);
//	setGameFieldValue(3, 1, 1);
//	setGameFieldValue(3, 2, 1);
//	setGameFieldValue(2, 3, 1);
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

//void printBoard(int8_t* board) {
//	for (int i = 0; i < GAME_DIMENSION; i += 1) {
//		for (int j = 0; j < GAME_DIMENSION; j += 1) {
//			printf("%02d ", board[i * GAME_DIMENSION + j]);
//		}
//		printf("\n");
//	}
//	printf("\n");
//}

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

static bool IRAM_ATTR displayGameState(void* args) {
	static uint8_t stepsToGeneration = 50;
	for (int i = 0; i < GAME_DIMENSION; i += 1) {
		for (int j = 0; j < GAME_DIMENSION; j += 1) {
			int8_t val = getGameFieldValue(i, j);
			if (val >= 20) {
				// fix living -> green
				gpLEDStrip->set_pixel(gpLEDStrip, i * GAME_DIMENSION + j, 0, val, 0);
			} else if (val < 0) {
				// dying -> red
				gpLEDStrip->set_pixel(gpLEDStrip, i * GAME_DIMENSION + j, -val, 0, 0);
				setGameFieldValue(i, j, val + 1);
			} else if (val == 0) {
				// fix dead -> dark
				gpLEDStrip->set_pixel(gpLEDStrip, i * GAME_DIMENSION + j, 0, 0, 0);
			} else {
				// starting life -> blue
				gpLEDStrip->set_pixel(gpLEDStrip, i * GAME_DIMENSION + j, 0, 0, val);
				setGameFieldValue(i, j, val + 1);
			}
		}
	}
	refreshDisplay = true;
	if (stepsToGeneration-- == 0) {
		stepsToGeneration = 50;
		calculateGeneration();
	}
	return false;
}

void app_main(void) {
	printf("Init LED\n");
    // Initialize WS2812 LED Strip with 25 pixels
    gpLEDStrip = led_strip_init(0, 8, 25);
    gpLEDStrip->clear(gpLEDStrip, 50);

    timer_config_t config = {
		.divider = (TIMER_BASE_CLK / 1000000), // result 80: 80 MHz -> 1 MHz (1 µs/Tick)
		.counter_dir = TIMER_COUNT_UP,
		.counter_en = TIMER_PAUSE,
		.alarm_en = TIMER_ALARM_EN,
		.auto_reload = TIMER_AUTORELOAD_EN,
	}; // default clock source is APB
	timer_init(TIMER_GROUP_0, TIMER_0, &config);
	timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0);
	timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, 20000); // 20 ms
	timer_enable_intr(TIMER_GROUP_0, TIMER_0);

	timer_isr_callback_add(TIMER_GROUP_0, TIMER_0, displayGameState, NULL, 0);

	generateGame();

	timer_start(TIMER_GROUP_0, TIMER_0);

	time_t now;
	char strftime_buf[64];
	struct tm timeinfo;

	time(&now);
	// Set timezone to China Standard Time
	setenv("TZ", "CET-1", 1);
	tzset();
	printf("BLA\n");
	localtime_r(&now, &timeinfo);
	strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
	printf("The current date/time in V is: %s\n", strftime_buf);

	printf("blu\n");
	while (1) {
        // update LEDs
		if (refreshDisplay) {
			gpLEDStrip->refresh(gpLEDStrip, 100);
		}
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
