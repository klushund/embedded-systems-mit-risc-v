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
#ifndef MAIN_TICKER_H_
#define MAIN_TICKER_H_

#include "driver/gpio.h"
#include "esp_err.h"

typedef enum {
	TickerEvent_RestartTicker
} TickerEvent;

typedef void (*TickerCallback)(TickerEvent);


esp_err_t ticker_init(gpio_num_t gpio);
void ticker_registerTickerCallback(TickerCallback tickerCallback);
void ticker_setText(char* tickerText, bool restartTicker);
void ticker_setDisplayBrightness(uint8_t brightness);
uint8_t ticker_getDefaultDisplayBrightness(void);

#endif /* MAIN_TICKER_H_ */
