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

.section .rodata
#caption:
#	.string "test: sum up n\n"
#sumtext:
#	.string "test: Sum from 1 to %d: %d\n"

.section .text
.global sum_up_func

sum_up_func:
	addi	 sp, sp, -16
	sw ra, 12(sp)
#	lui a0, %hi(caption)
#	addi a0, a0, %lo(caption)
#	jal ra, puts
	li a5, 1
	li a2, 0
countloop:
	li a4, 100
	bltu a4, a5, outofcountloop
	add a2, a2, a5
	addi a5, a5, 1
	j countloop
outofcountloop:
#	li a1, 100
	#li a0, sumtext
#	jal ra, printf
	mv a0, a2
	lw ra, 12(sp)
	addi sp, sp, 16
	ret
