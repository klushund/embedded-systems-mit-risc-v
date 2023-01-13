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

#ifndef FILTER_FILTER_H_
#define FILTER_FILTER_H_

typedef struct _Filter_ {
	void (*destroy)(struct _Filter_* pFilter);
	void (*reset)(struct _Filter_* pFilter);
	float (*filterValue)(struct _Filter_* pFilter, float value);
} Filter;

void filter_destroy(Filter* pFilter);
void filter_reset(Filter* pFilter);
float filter_filterValue(Filter* pFilter, float value);

#endif /* FILTER_FILTER_H_ */
