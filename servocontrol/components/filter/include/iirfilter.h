/*
 * Example of the book "Embedded Systems mit RISC-V", dpunkt.verlag
 * Author: Patrick Ritschel
 *
 * see https://ritschel.at/kapitel-8-5-4-pulsweitenmodulation-pwm-applikation-servocontrol/
 *
 * The code of this project is in the Public Domain (or CC0 licensed, at your option).
 * Unless required by applicable law or agreed to in writing, this
 * software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied.
 */

#ifndef FILTER_IIRFILTER_H_
#define FILTER_IIRFILTER_H_

#include "filter.h"

Filter* iirfilter_create(float a0, float a1, float b0, float b1, float b2);

#endif /* FILTER_IIRFILTER_H_ */
