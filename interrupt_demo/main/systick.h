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

#ifndef MAIN_SYSTICK_H_
#define MAIN_SYSTICK_H_

#include <stdbool.h>
#include <unistd.h>

void systick_init(void);
uint64_t getSysticks(void);


#endif /* MAIN_SYSTICK_H_ */
