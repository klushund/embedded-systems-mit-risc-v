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

#ifndef RINGBUFFER_RINGBUFFER_H_
#define RINGBUFFER_RINGBUFFER_H_

#include <inttypes.h>
#include <stdbool.h>

#define RINGBUFFER_SUCCESS				0
#define RINGBUFFER_ERROR_EMPTY			-1
#define RINGBUFFER_ERROR_FULL			-2
#define RINGBUFFER_ERROR_OUTOFMEMORY	-3

typedef int32_t RingbufferHandle;

/**
 * Creates a new ringbuffer by allocating space for >size< elements in dynamic memory.
 * @param size number of queue elements
 * @return RINGBUFFER_ERROR_OUTOFMEMORY upon failure or a positive valued handle upon success
 */
RingbufferHandle ringbuffer_create(uint32_t size);
void ringbuffer_destroy(RingbufferHandle* pRingBufferHandle);
void ringbuffer_clear(RingbufferHandle ringbufferHandle);
bool ringbuffer_isEmpty(RingbufferHandle ringbufferHandle);
bool ringbuffer_isFull(RingbufferHandle ringbufferHandle);
void ringbuffer_addFloat(RingbufferHandle ringbufferHandle, float value);
bool ringbuffer_getFloat(RingbufferHandle ringbufferHandle, float* pValue, uint32_t index);

#endif /* RINGBUFFER_RINGBUFFER_H_ */
