/*
 * Example of the book "Embedded Systems mit RISC-V", dpunkt.verlag
 * Author: Patrick Ritschel
 *
 * see https://ritschel.at/kapitel-6-interrupts-und-exceptions-beispiel-interrupt_demo/
 *
 * The code of this project is in the Public Domain (or CC0 licensed, at your option).
 * Unless required by applicable law or agreed to in writing, this
 * software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied.
 */

#include "driver/gptimer.h"
#include "systick.h"

static uint64_t gSysticks;

static bool timerAlarmCallback(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx);

gptimer_event_callbacks_t gTimerCallbacks = {
    .on_alarm = timerAlarmCallback, // register user callback
};

// ***** implementation *****
void systick_init() {
	// setup timer to increment gSysticks each millisecond
	gptimer_handle_t timer = NULL;
	gptimer_config_t timerConfig = {
	    .clk_src = GPTIMER_CLK_SRC_DEFAULT,
	    .direction = GPTIMER_COUNT_UP,
	    .resolution_hz = 1 * 1000 * 1000, // 1 MHz, 1 tick = 1 us
	};
	ESP_ERROR_CHECK(gptimer_new_timer(&timerConfig, &timer));
	ESP_ERROR_CHECK(gptimer_register_event_callbacks(timer, &gTimerCallbacks, NULL));
	gptimer_alarm_config_t alarmConfig = {
	    .reload_count = 0, // counter will reload with 0 on alarm event
	    .alarm_count = 1000, // each ms
	    .flags.auto_reload_on_alarm = true, // enable auto-reload
	};
	ESP_ERROR_CHECK(gptimer_set_alarm_action(timer, &alarmConfig));
	ESP_ERROR_CHECK(gptimer_enable(timer));
	ESP_ERROR_CHECK(gptimer_start(timer));
}

static bool timerAlarmCallback(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx) {
	// tell the compiler that the parameters are intentionally not used
	(void) timer;
	(void) edata;
	(void) user_ctx;
    gSysticks += 1;
    return false; // no need to yield to other tasks immediately after the callback
}

uint64_t getSysticks() {
	uint32_t intstate;
	// save mstatus register in intstate and clear MIE
	asm volatile (" csrrci %0, mstatus, 0x8" : "=r"(intstate));
	// critical section
	uint64_t systicks = gSysticks;
	// restore mstatus from intstate
	asm volatile (" csrw mstatus, %0" : : "r"(intstate));
	return systicks;
}

