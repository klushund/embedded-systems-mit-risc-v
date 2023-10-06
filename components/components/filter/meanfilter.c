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

#include "meanfilter.h"

#include <stdio.h>
#include <malloc.h>
#include <memory.h>

typedef struct _MeanFilter_ {
	Filter filter;
	uint32_t order;
	uint32_t offset;
	float* buffer;
	float sum;
} MeanFilter;

static void meanfilter_destroy(Filter* pFilter);
static void meanfilter_reset(Filter* pFilter);
float meanfilter_filterValue(Filter* pFilter, float value);

Filter* meanfilter_create(uint32_t order) {
	MeanFilter* pMeanFilter = malloc(sizeof(MeanFilter));
	if (pMeanFilter == NULL) {
		return NULL;
	}
	pMeanFilter->order = order;
	if ((pMeanFilter->buffer = malloc(order * sizeof(float))) == NULL) {
		free(pMeanFilter);
		return NULL;
	}
	meanfilter_reset((Filter*)pMeanFilter);
	// set function pointers
	pMeanFilter->filter.destroy = meanfilter_destroy;
	pMeanFilter->filter.reset = meanfilter_reset;
	pMeanFilter->filter.filterValue = meanfilter_filterValue;
	return (Filter*)pMeanFilter;
}

void meanfilter_destroy(Filter* pFilter) {
	MeanFilter* pMeanFilter = (MeanFilter*)pFilter;
	free(pMeanFilter->buffer);
	free(pMeanFilter);
}

void meanfilter_reset(Filter* pFilter) {
	MeanFilter* pMeanFilter = (MeanFilter*)pFilter;
	pMeanFilter->offset = 0;
	pMeanFilter->sum = 0.0f;
	for (int i = 0; i < pMeanFilter->order; i += 1) {
		pMeanFilter->buffer[i] = 0.0f;
	}
}

float meanfilter_filterValue(Filter* pFilter, float value) {
	MeanFilter* pMeanFilter = (MeanFilter*)pFilter;
	// overwrite value
	pMeanFilter->sum -= pMeanFilter->buffer[pMeanFilter->offset];
	pMeanFilter->buffer[pMeanFilter->offset] = value;
	pMeanFilter->sum += value;
	pMeanFilter->offset = (pMeanFilter->offset + 1) % pMeanFilter->order;
	float avg = pMeanFilter->sum / pMeanFilter->order;
	return avg - value;
}
