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

#include <stdio.h>
#include <malloc.h>
#include <memory.h>

#include "ringbuffer.h"
#include "firfilter.h"

typedef struct _FIRFilter_ {
	Filter filter;
	float* b;
	size_t blen;
	RingbufferHandle ringbufferHandle;
} FIRFilter;

static void firfilter_destroy(Filter* pFilter);
static void firfilter_reset(Filter* pFilter);
float firfilter_filterValue(Filter* pFilter, float value);

// b are the filter coefficients
Filter* firfilter_create(float* b, size_t blen) {
	FIRFilter* pFIRFilter = malloc(sizeof(FIRFilter));
	if (pFIRFilter == NULL) {
		return NULL;
	}
	if ((pFIRFilter->b = malloc(sizeof(float) * blen)) == NULL) {
		free(pFIRFilter);
		return NULL;
	}
	pFIRFilter->blen = blen;
	memcpy(pFIRFilter->b, b, blen * sizeof(float));
	if ((pFIRFilter->ringbufferHandle = ringbuffer_create(blen)) < RINGBUFFER_SUCCESS) {
		free(pFIRFilter->b);
		free(pFIRFilter);
		return 0;
	}
	// set function pointers
	pFIRFilter->filter.destroy = firfilter_destroy;
	pFIRFilter->filter.reset = firfilter_reset;
	pFIRFilter->filter.filterValue = firfilter_filterValue;
	return (Filter*)pFIRFilter;
}

void firfilter_destroy(Filter* pFilter) {
	FIRFilter* pFIRFilter = (FIRFilter*)pFilter;
	ringbuffer_destroy(&pFIRFilter->ringbufferHandle);
	free(pFIRFilter->b);
	free(pFIRFilter);
}

void firfilter_reset(Filter* pFilter) {
	FIRFilter* pFIRFilter = (FIRFilter*)pFilter;
	ringbuffer_clear(pFIRFilter->ringbufferHandle);
}

float firfilter_filterValue(Filter* pFilter, float value) {
	FIRFilter* pFIRFilter = (FIRFilter*)pFilter;
	ringbuffer_addFloat(pFIRFilter->ringbufferHandle, value);
	// Calculate new output value
	float y = 0;
	for (uint32_t i = 0; i < pFIRFilter->blen; i += 1) {
		float x;
		if (!ringbuffer_getFloat(pFIRFilter->ringbufferHandle, &x, i)) {
			return 0;
		}
		y += (pFIRFilter->b[i] * x);
	}
	return y;
}
