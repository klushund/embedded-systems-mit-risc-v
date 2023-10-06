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

#include "esp_log.h"
#include "lwip/err.h"
#include "lwip/sockets.h"

static const char* TAG = "sendpacket";

void sendpacket_sendTCP(char* hostIP, uint16_t port, uint8_t* payload, uint16_t payloadLen) {
	int sock =  socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	if (sock < 0) {
		ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
		return;
	}
	ESP_LOGI(TAG, "Socket created, connecting to %s:%d", hostIP, port);

	struct sockaddr_in dest_addr;
	dest_addr.sin_addr.s_addr = inet_addr(hostIP);
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(port);
	int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr_in6));
	if (err != 0) {
		ESP_LOGE(TAG, "Socket unable to connect: errno %d", errno);
		return;
	}
	ESP_LOGI(TAG, "Successfully connected");

	err = send(sock, payload, payloadLen, 0);
	if (err < 0) {
		ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
		return;
	}

	ESP_LOGE(TAG, "Closing socket...");
	shutdown(sock, 0);
	close(sock);
}

void sendpacket_sendUDP(char* hostIP, uint16_t port, uint8_t* payload, uint16_t payloadLen) {
	int sock =  socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock < 0) {
		ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
		return;
	}
	ESP_LOGI(TAG, "Socket created, sending datagram to %s:%d", hostIP, port);

	struct sockaddr_in dest_addr;
	dest_addr.sin_addr.s_addr = inet_addr(hostIP);
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(port);
	int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr_in6));
	if (err != 0) {
		ESP_LOGE(TAG, "Socket unable to connect: errno %d", errno);
		return;
	}
	ESP_LOGI(TAG, "Successfully connected");

	err = send(sock, payload, payloadLen, 0);
	if (err < 0) {
		ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
		return;
	}

	ESP_LOGE(TAG, "Closing socket...");
	shutdown(sock, 0);
	close(sock);
}
