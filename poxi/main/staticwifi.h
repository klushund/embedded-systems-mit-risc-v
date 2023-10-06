/*
 * Example of the book "Embedded Systems mit RISC-V und ESP32-C3", dpunkt.verlag
 * Author: Patrick Ritschel
 *
 * see https://ritschel.at/buchprojekt-pulsoximeter/
 *
 * This module sets up WIFI in station mode with a static IP address.
 *
 * The code of this project is in the Public Domain (or CC0 licensed, at your option).
 * Unless required by applicable law or agreed to in writing, this
 * software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied.
 */

#ifndef MAIN_STATICWIFI_H_
#define MAIN_STATICWIFI_H_

#include "sdkconfig.h"

#ifndef CONFIG_USE_PROVISIONING
#include "esp_err.h"

esp_err_t staticwifi_init(void);

#endif // CONFIG_USE_PROVISIONING

#endif /* MAIN_STATICWIFI_H_ */
