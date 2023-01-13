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

#ifndef MAIN_BUTTON_H_
#define MAIN_BUTTON_H_

#if CONFIG_CALLBACK_NONE == 1
extern bool gKeypressed;
#elif CONFIG_CALLBACK_DYNAMIC == 1
typedef void (*KeyCallback)(uint8_t key);
#endif

void button_init(void);

#if CONFIG_CALLBACK_DYNAMIC == 1
void registerKeyCallback(KeyCallback keyCallback);
#endif

#endif /* MAIN_BUTTON_H_ */
