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

#include <malloc.h>

#include "filter.h"

void filter_destroy(Filter* pFilter) {
	free(pFilter);
}

void filter_reset(Filter* pFilter) {
	pFilter->reset(pFilter);
}

float filter_filterValue(Filter* pFilter, float value) {
	return pFilter->filterValue(pFilter, value);
}
