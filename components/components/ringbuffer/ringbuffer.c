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
#include <stddef.h>
#include <malloc.h>

#include "ringbuffer.h"

typedef struct _Ringbuffer_ {
	uint32_t writeOffset;
	uint32_t count;
	size_t size;
	float* values;
} Ringbuffer;

// creates a Ringbuffer with size uint32_t Elements
RingbufferHandle ringbuffer_create(uint32_t size) {
	Ringbuffer* pRingbuffer = malloc(sizeof(Ringbuffer));
	if (pRingbuffer == NULL) {
		return RINGBUFFER_ERROR_OUTOFMEMORY;
	}
	if ((pRingbuffer->values = malloc(size * sizeof(float))) == NULL) {
		free(pRingbuffer);
		return RINGBUFFER_ERROR_OUTOFMEMORY;
	}
	pRingbuffer->size = size;
	pRingbuffer->writeOffset = 0;
	pRingbuffer->count = 0;
	return (RingbufferHandle) pRingbuffer;
}

void ringbuffer_destroy(RingbufferHandle* pRingbufferHandle) {
	Ringbuffer* pRingbuffer = (Ringbuffer*) *pRingbufferHandle;
	free(pRingbuffer->values);
	free(*((Ringbuffer**)pRingbufferHandle));
	*pRingbufferHandle = 0;
}

void ringbuffer_clear(RingbufferHandle ringbufferHandle) {
	Ringbuffer* pRingbuffer = (Ringbuffer*) ringbufferHandle;
	pRingbuffer->writeOffset = 0;
	pRingbuffer->count = 0;
}

bool ringbuffer_isEmpty(RingbufferHandle ringbufferHandle) {
	Ringbuffer* pRingbuffer = (Ringbuffer*) ringbufferHandle;
	return (pRingbuffer->count == 0);
}

bool ringbuffer_isFull(RingbufferHandle ringbufferHandle) {
	Ringbuffer* pRingbuffer = (Ringbuffer*) ringbufferHandle;
	return (pRingbuffer->count >= pRingbuffer->size);
}

void ringbuffer_addFloat(RingbufferHandle ringbufferHandle, float value) {
	Ringbuffer* pRingbuffer = (Ringbuffer*) ringbufferHandle;
	pRingbuffer->values[pRingbuffer->writeOffset] = value;
	pRingbuffer->writeOffset = (pRingbuffer->writeOffset + 1) % pRingbuffer->size;
	if (pRingbuffer->count < pRingbuffer->size) {
		pRingbuffer->count += 1;
	}
}

bool ringbuffer_getFloat(RingbufferHandle ringbufferHandle, float* pValue, uint32_t index) {
	Ringbuffer* pRingbuffer = (Ringbuffer*) ringbufferHandle;
	if (index >= pRingbuffer->count) {
		return false;
	}
	int32_t offs = pRingbuffer->writeOffset - pRingbuffer->count + index;
	if (offs < 0) {
		offs += pRingbuffer->size;
	} else {
		offs %= pRingbuffer->size;
	}
	*pValue = pRingbuffer->values[offs];
	return true;
}
