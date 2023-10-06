/*
 * Example of the book "Embedded Systems mit RISC-V und ESP32-C3", dpunkt.verlag
 * Author: Patrick Ritschel
 *
 * see https://ritschel.at/buchprojekt-pulsoximeter/
 *
 * This module contains the MQTT client implementation.
 *
 * The code of this project is in the Public Domain (or CC0 licensed, at your option).
 * Unless required by applicable law or agreed to in writing, this
 * software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied.
 */

#ifndef MAIN_MQTT_H_
#define MAIN_MQTT_H_

void mqtt_init(void);
void sendpacket_sendMQTT(uint8_t* payload, uint16_t payloadLen);

#endif /* MAIN_MQTT_H_ */
