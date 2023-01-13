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
#include <inttypes.h>

void app_main(void) {
    printf("sum up n\n");

    const uint32_t upperNumber = 100;
    uint32_t sum = 0;

    // add the values in a loop
    uint32_t i = 1;
    while (i <= upperNumber) {
    	sum += i;
    	i += 1;
    }

    printf("Sum of 1 to %lu: %lu\n", upperNumber, sum);
}
