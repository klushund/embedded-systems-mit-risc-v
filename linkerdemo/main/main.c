/*
 * Example of the book "Embedded Systems mit RISC-V", dpunkt.verlag
 * Author: Patrick Ritschel
 *
 * see https://ritschel.at/kapitel-4-2-4-linker-beispiel-linkerdemo/
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
#include <esp_attr.h>
#include <esp_system.h>

// "persistent" global variables
__attribute__((section(".noinit"))) uint32_t gKeepValue;
__NOINIT_ATTR uint32_t gKeepValue2;
RTC_NOINIT_ATTR uint32_t gDeepKeepValue;

// fetch adresses from linker
extern uint32_t _rtc_noinit_start;
extern uint32_t _rtc_noinit_end;

void app_main() {
	printf("Reset occurred, starting linkerdemo...\n");
	uint32_t* pRTCNoinitStart = &_rtc_noinit_start;
	uint32_t* pRTCNoinitEnd = &_rtc_noinit_end;
	printf("rtc noinit start 0x%8p, rtc noinit end 0x%8p\n", pRTCNoinitStart, pRTCNoinitEnd);

	while (true) {
        printf("gKeepValue: %ld, gKeepValue2: %ld, gDeepKeepValue: %ld\n", gKeepValue++, gKeepValue2++, gDeepKeepValue++);
        printf("Try a reset.\n");
        esp_restart();
    }
}
