/*
 * Example of the book "Embedded Systems mit RISC-V", dpunkt.verlag
 * Author: Patrick Ritschel
 *
 * see https://ritschel.at/kapitel-4-2-3-cache-beispiel-histogram/
 *
 * Based on the basic Espressif IDF-project.
 *
 * The code of this project is in the Public Domain (or CC0 licensed, at your option).
 * Unless required by applicable law or agreed to in writing, this
 * software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied.
 */

#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

#define CSR_MSTATUS					0x300
#define CSR_MSTATUS_MIE				0x8

#define EVENTTYPE_CYCLES			0x01
#define EVENTTYPE_INSTRUCTIONS		0x02

#define TESTSIZE					(CONFIG_TESTSIZE_KiB * 1024)
#define BLOCKSIZE					32

#include "nibelungen.c"

static void disableInterrupts(void);

// ***** implementation *****

void disableInterrupts(void) {
	asm volatile ("csrci 0x300, 0x8" : : );
    // check whether switching off worked
	uint32_t csrval;
	asm volatile ("	csrr %0, 0x300" : "=r"(csrval));
	if (csrval & CSR_MSTATUS_MIE) {
    	printf("Switching off interrupts failed!\n");
    	while (true)
    		; // "eternal" loop
    }
}

void app_main() {
	// disable interrupts and initialize measurement
	disableInterrupts();
	#if CONFIG_MEASURE_CYCLES == 1
		uint32_t eventType = EVENTTYPE_CYCLES;
		char* measureType = "clock cycles";
	#else
		uint32_t eventType = EVENTTYPE_INSTRUCTIONS;
		char* measureType = "instructions";
	#endif

	uint32_t csrval = 0;
	asm volatile (" csrw 0x7E0, %0" : : "r"(eventType));
	asm volatile (" csrwi 0x7E2, %0" : : "rK"(csrval));

	// calculate histogram
	uint32_t histogram[256] = { 0 };
	uint32_t offs = 0;
	for (uint32_t i = 0; i < TESTSIZE; i += 1) {
		histogram[(unsigned char)gNibelungenlied[offs]] += 1;
		offs += BLOCKSIZE;
		if (offs >= TESTSIZE) {
			offs -= TESTSIZE;
			offs += 1;
		}
	}

	// print measurement
	asm volatile ("	csrr %0, 0x7E2" : "=r"(csrval));
	printf("Test of %d KiB took %ld %s\n", CONFIG_TESTSIZE_KiB, csrval, measureType);
}
