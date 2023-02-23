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

#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include "esp_log.h"
#include "ticker.h"

#define TAG "tickerapp"

// ***** implementation *****
void app_main() {
	ESP_LOGI(TAG, "Starting tickerapp");
	ESP_ERROR_CHECK(ticker_init(CONFIG_LED_STRIP_GPIO));
	ticker_setText("\1rWelcome \1gto \1bthe\1o>>ticker<<\1w!!! ", true);
    while (true) {
        printf("Ticker is running.\n");
        sleep(60);
    }
}
