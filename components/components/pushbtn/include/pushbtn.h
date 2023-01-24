/*
 * Part of the book "Embedded Systems mit RISC-V", dpunkt.verlag
 * Author: Patrick Ritschel
 *
 * see https://ritschel.at/buch-embedded-systems-auf-den-punkt-gebracht/
 *
 * The code of this project is in the Public Domain (or CC0 licensed, at your option).
 * Unless required by applicable law or agreed to in writing, this
 * software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied.
 */
#include "freertos/message_buffer.h"

enum ButtonState {
	ButtonState_Pressed = 1,
	ButtonState_Released
};

struct ButtonEvent {
	uint64_t systemtime;
	enum ButtonState buttonState;
};

void pushbtn_init();
void pushbtn_registerObserver(MessageBufferHandle_t observerMB);
