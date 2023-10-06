/*
 * Example of the book "Embedded Systems mit RISC-V und ESP32-C3", dpunkt.verlag
 * Author: Patrick Ritschel
 *
 * see https://ritschel.at/buchprojekt-pulsoximeter/
 *
 * This module contains the implementation of device provisioning based in BLE.
 * The code is mainly taken from wifi_prov_mgr ESP-IDF example.
 *
 * The code of this project is in the Public Domain (or CC0 licensed, at your option).
 * Unless required by applicable law or agreed to in writing, this
 * software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied.
 */

#ifndef MAIN_PROVISIONING_H_
#define MAIN_PROVISIONING_H_

#include "sdkconfig.h"

#if CONFIG_USE_PROVISIONING

void provisioning_init(void);

#endif // CONFIG_USE_PROVISIONING

#endif /* MAIN_PROVISIONING_H_ */
