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

#include "dcfilter.h"

#include <stdio.h>
#include <malloc.h>
#include <memory.h>

typedef struct _DCFilter_ {
	Filter filter;
	float alpha;
	float w;
} DCFilter;

static void dcfilter_destroy(Filter* pFilter);
static void dcfilter_reset(Filter* pFilter);
float dcfilter_filterValue(Filter* pFilter, float value);

// alpha is the filter coefficient
Filter* dcfilter_create(float alpha) {
	DCFilter* pDCFilter = malloc(sizeof(DCFilter));
	if (pDCFilter == NULL) {
		return NULL;
	}
	dcfilter_reset((Filter*)pDCFilter);
	pDCFilter->alpha = alpha;
	// set function pointers
	pDCFilter->filter.destroy = dcfilter_destroy;
	pDCFilter->filter.reset = dcfilter_reset;
	pDCFilter->filter.filterValue = dcfilter_filterValue;
	return (Filter*)pDCFilter;
}

void dcfilter_destroy(Filter* pFilter) {
	free(pFilter);
}

void dcfilter_reset(Filter* pFilter) {
	DCFilter* pDCFilter = (DCFilter*)pFilter;
	pDCFilter->w = 0.0f;
}

float dcfilter_filterValue(Filter* pFilter, float value) {
	DCFilter* pDCFilter = (DCFilter*)pFilter;
	float w = value + pDCFilter->alpha * pDCFilter->w;
	float retval = w - pDCFilter->w;
	pDCFilter->w = w;
	return retval;
}
