/*
 * Example of the book "Embedded Systems mit RISC-V", dpunkt.verlag
 * Author: Patrick Ritschel
 *
 * see https://ritschel.at/kapitel-3-1-prozessorarchitektur-beispiel-sum_up_n/
 *
 * Based on ADC-Example from ESP-IDF.
 *
 * The code of this project is in the Public Domain (or CC0 licensed, at your option).
 * Unless required by applicable law or agreed to in writing, this
 * software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied.
 */
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

#define USE_GAUSS_FORMULA			0 // 0: no, 1: variant 1, 2: variant 2

#if CONFIG_USE_CSR_MACROS == 1
#include <riscv/csr.h>
#endif

#define CSR_MSTATUS					0x300
#define CSR_MSTATUS_MIE				0x8

#define CSR_MPCER					0x7E0
#define CSR_MPCMR					0x7E1
#define CSR_MPCCR					0x7E2

#if CONFIG_USE_ASSEMBLER_FUNCTION == 1
extern uint32_t sum_up_func(uint32_t upperNumber);
#endif

// switch off global interrupt enable
void disableInterrupts(void) {
#if CONFIG_USE_CSR_MACROS == 0
	asm volatile ("csrci 0x300, 0x8" : : );
#else
	RV_CLEAR_CSR(CSR_MSTATUS, CSR_MSTATUS_MIE);
#endif
    // check whether switching off worked
#if CONFIG_USE_CSR_MACROS == 0
	uint32_t csrval;
	asm volatile ("	csrr %0, 0x300" : "=r"(csrval));
	if (csrval & CSR_MSTATUS_MIE) {
#else
    if (RV_READ_CSR(CSR_MSTATUS) & CSR_MSTATUS_MIE) {
#endif
    	printf("Switching off interrupts failed!\n");
    	while (true)
    		; // "eternal" loop
    }
}

#if (CONFIG_CALCULATE_IN_FUNCTION == 1) && (CONFIG_USE_ASSEMBLER_FUNCTION == 0)
uint32_t sum_up_n(uint32_t upperNumber) {
	return sum_up_func(upperNumber);
#if (CONFIG_GAUSS_FORMULA_1 == 1)
	return (upperNumber * (upperNumber + 1)) / 2;
#elif (CONFIG_GAUSS_FORMULA_2 == 1)
	return ((upperNumber * upperNumber) + upperNumber) / 2;
#else
	// add the values in a loop
	uint32_t i = 1;
	uint32_t sum = 0;
	while (i <= upperNumber) {
		sum += i;
		i += 1;
	}
	return sum;
#endif
}
#endif

#if (CONFIG_USE_LOOP == 1)
#define NUMBER_OF_COUNTER_EVENTS	11
#define NUMBER_OF_TESTS				8

#ifndef CONFIG_USE_CONSTANT_TEST_VALUE
uint32_t upperNumbers[NUMBER_OF_TESTS] = { 1, 1, 10, 100, 1000, 10000, 100000, 1000000 };
#endif
char* counterEventNames[NUMBER_OF_COUNTER_EVENTS] = { "CYCLE", "INST", "LD_HAZARD", "JMP_HAZARD", "IDLE", "LOAD", "STORE", "JMP_UNCOND", "BRANCH", "BRANCH_TAKEN", "INST_COMP" };
uint32_t measurements[NUMBER_OF_TESTS][NUMBER_OF_COUNTER_EVENTS];
#endif

//extern uint32_t test_main(void);
void app_main(void) {
    printf("sum up n\n");

#if (CONFIG_USE_LOOP == 1)
    uint32_t sum = 0;
    uint32_t csrval;
    for (int i = 0; i < NUMBER_OF_TESTS; i += 1) {
#ifdef CONFIG_USE_CONSTANT_TEST_VALUE
    	const uint32_t upperNumber = CONFIG_USE_CONSTANT_TEST_VALUE;
#else
		uint32_t upperNumber = upperNumbers[i];
#endif

    	for (int j = 0; j < NUMBER_OF_COUNTER_EVENTS; j += 1) {
			uint32_t eventType = (0x1 << j);

#if CONFIG_USE_CSR_MACROS == 0
			asm volatile (" csrw 0x7E0, %0" : : "r"(eventType));
			// reset counter by writing 0 to mpccr
			csrval = 0;
			asm volatile (" csrwi 0x7E2, %0" : : "rK"(csrval));
#else
    		RV_WRITE_CSR(CSR_MPCER, eventType);
    		RV_WRITE_CSR(CSR_MPCCR, 0);
#endif
			sum = 0;
#else
    		const uint32_t upperNumber = 100;
    	    uint32_t sum = 0;
#endif

#if CONFIG_CALCULATE_IN_FUNCTION == 1
	#if CONFIG_USE_ASSEMBLER_FUNCTION == 1
    	    sum = sum_up_func(upperNumber);
	#else
    		sum = sum_up_n(upperNumber);
	#endif
#else
#if (CONFIG_GAUSS_FORMULA_1 == 1)
    		sum = (upperNumber * (upperNumber + 1)) / 2;
#elif (CONFIG_GAUSS_FORMULA_2 == 1)
    		sum = ((upperNumber * upperNumber) + upperNumber) / 2;
#else
    		// add the values in a loop
			uint32_t k = 1;
			while (k <= upperNumber) {
				sum += k;
				k += 1;
			}
#endif
#endif

#if (CONFIG_USE_LOOP == 1)
#if CONFIG_USE_CSR_MACROS == 0
			asm volatile ("	csrr %0, 0x7E2" : "=r"(csrval));
#else
    		csrval = RV_READ_CSR(CSR_MPCCR);
#endif
    		measurements[i][j] = csrval;
    		printf("Sum of 1 to %d: %u; csr: %d\n", upperNumber, sum, csrval);
    	}
    }

    printf("Measurements\nupperNumber\t");
    for (int j = 0; j < NUMBER_OF_COUNTER_EVENTS; j+= 1) {
    	printf("%s\t", counterEventNames[j]);
    }
    printf("\n");
	for (int i = 0; i < NUMBER_OF_TESTS; i += 1) {
#ifdef CONFIG_USE_CONSTANT_TEST_VALUE
		printf("%d\t", CONFIG_USE_CONSTANT_TEST_VALUE);
#else
		printf("%d\t", upperNumbers[i]);
#endif
		for (int j = 0; j < NUMBER_OF_COUNTER_EVENTS; j+= 1) {
    		printf("%d\t", measurements[i][j]);
    	}
		printf("\n");
    }
#else
	printf("Sum of 1 to %lu: %lu\n", upperNumber, sum);
#endif
}
