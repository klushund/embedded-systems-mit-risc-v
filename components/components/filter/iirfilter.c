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
#include <memory.h>

#include "ringbuffer.h"
#include "iirfilter.h"

typedef struct _IIRFilter_ {
	Filter filter;
	float a[2];
	float b[3];
	float w[3];
} IIRFilter;

static void iirfilter_destroy(Filter* pFilter);
static void iirfilter_reset(Filter* pFilter);
static float iirfilter_filterValue(Filter* pFilter, float value);

// b are the filter coefficients
Filter* iirfilter_create(float a0, float a1, float b0, float b1, float b2) {
	IIRFilter* pIIRFilter = malloc(sizeof(IIRFilter));
	if (pIIRFilter == NULL) {
		return NULL;
	}
	pIIRFilter->a[0] = a0;
	pIIRFilter->a[1] = a1;
	pIIRFilter->b[0] = b0;
	pIIRFilter->b[1] = b1;
	pIIRFilter->b[2] = b2;
	pIIRFilter->w[0] = 0.0;
	pIIRFilter->w[1] = 0.0;
	pIIRFilter->w[2] = 0.0;
	// set function pointers
	pIIRFilter->filter.destroy = iirfilter_destroy;
	pIIRFilter->filter.reset = iirfilter_reset;
	pIIRFilter->filter.filterValue = iirfilter_filterValue;
	return (Filter*)pIIRFilter;
}

void iirfilter_destroy(Filter* pFilter) {
	free(pFilter);
}

void iirfilter_reset(Filter* pFilter) {
	IIRFilter* pIIRFilter = (IIRFilter*)pFilter;
	pIIRFilter->w[0] = 0.0;
	pIIRFilter->w[1] = 0.0;
	pIIRFilter->w[2] = 0.0;
}

static float iirfilter_filterValue(Filter* pFilter, float value) {
	IIRFilter* pIIRFilter = (IIRFilter*)pFilter;
	pIIRFilter->w[0] = value + (pIIRFilter->a[0] * pIIRFilter->w[1]) + (pIIRFilter->a[1] * pIIRFilter->w[2]);
	float y = (pIIRFilter->b[0] * pIIRFilter->w[0]) + (pIIRFilter->b[1] * pIIRFilter->w[1]) +
			(pIIRFilter->b[2] * pIIRFilter->w[2]);
	pIIRFilter->w[2] = pIIRFilter->w[1];
	pIIRFilter->w[1] = pIIRFilter->w[0];
	return y;
}
