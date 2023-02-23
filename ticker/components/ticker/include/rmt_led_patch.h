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
#ifndef COMPONENTS_TICKER_INCLUDE_RMT_LED_PATCH_H_
#define COMPONENTS_TICKER_INCLUDE_RMT_LED_PATCH_H_

#include "led_strip_interface.h"
#include "led_strip_types.h"
#include "led_strip_rmt.h"
#include "driver/rmt_types.h"
#include "driver/rmt_common.h"

esp_err_t rmt_led_patch_changeChannel(const led_strip_config_t *led_config, const led_strip_rmt_config_t *rmt_config, led_strip_t *strip);

#endif /* COMPONENTS_TICKER_INCLUDE_RMT_LED_PATCH_H_ */
