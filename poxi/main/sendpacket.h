/*
 * Example of the book "Embedded Systems mit RISC-V und ESP32-C3", dpunkt.verlag
 * Author: Patrick Ritschel
 *
 * see https://ritschel.at/buchprojekt-pulsoximeter/
 *
 * This module contains two functions, one for sending a pulse entry using TCP/IP and the
 * other one using UDP/IP.
 *
 * The code of this project is in the Public Domain (or CC0 licensed, at your option).
 * Unless required by applicable law or agreed to in writing, this
 * software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied.
 */

#ifndef MAIN_SENDPACKET_H_
#define MAIN_SENDPACKET_H_

void sendpacket_sendTCP(char* hostIP, uint16_t port, uint8_t* payload, uint16_t payloadLen);
void sendpacket_sendUDP(char* hostIP, uint16_t port, uint8_t* payload, uint16_t payloadLen);

#endif /* MAIN_SENDPACKET_H_ */
