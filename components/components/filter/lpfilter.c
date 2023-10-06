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

#include "lpfilter.h"

#include <stdio.h>
#include <malloc.h>
#include <memory.h>

typedef struct _LPFilter_ {
	Filter filter;
	float alpha;
	float value;
} LPFilter;

static void lpfilter_destroy(Filter* pFilter);
static void lpfilter_reset(Filter* pFilter);
float lpfilter_filterValue(Filter* pFilter, float value);

// alpha is the filter coefficient
Filter* lpfilter_create(float alpha) {
	LPFilter* pLPFilter = malloc(sizeof(LPFilter));
	if (pLPFilter == NULL) {
		return NULL;
	}
	lpfilter_reset((Filter*)pLPFilter);
	pLPFilter->alpha = alpha;
	// set function pointers
	pLPFilter->filter.destroy = lpfilter_destroy;
	pLPFilter->filter.reset = lpfilter_reset;
	pLPFilter->filter.filterValue = lpfilter_filterValue;
	return (Filter*)pLPFilter;
}

void lpfilter_destroy(Filter* pFilter) {
	free(pFilter);
}

void lpfilter_reset(Filter* pFilter) {
	LPFilter* pLPFilter = (LPFilter*)pFilter;
	pLPFilter->value = 0.0f;
}

float lpfilter_filterValue(Filter* pFilter, float value) {
	LPFilter* pLPFilter = (LPFilter*)pFilter;

	//p[n]= alpha * p[n−1]+(1−alpha)pi[n]: LPF with cutoff freq < 4Hz = fs * (1-alpha) / (2 PI alpha)
	pLPFilter->value = pLPFilter->alpha * pLPFilter->value + (1.0f - pLPFilter->alpha) * value;
	return pLPFilter->value;
}
