/*
 * Example of the book "Embedded Systems mit RISC-V", dpunkt.verlag
 * Author: Patrick Ritschel
 *
 * see https://ritschel.at/kapitel-5-4-led-schalten-beispiel-switch_led/
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
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// peripheral definitions
#define GPIO_BASE 						0x60004000
#define GPIO_ENABLE_REG_OFFS			0x0020
#define gpioEnableReg					*((volatile uint32_t*)(GPIO_BASE | GPIO_ENABLE_REG_OFFS))
#define GPIO_OUT_REG_OFFS 				0x0004
#define gpioOutReg 						*((volatile uint32_t*)(GPIO_BASE | GPIO_OUT_REG_OFFS))

#define IO_MUX_BASE 					0x60009000
#define IO_MUX_GPIOn_REG_OFFS(n) 		(0x0004 + n * 4)
#define IO_MUX_PIN_FUNC_GPIO 			1
#define IO_MUX_PIN_DRV_40mA 			3
#if (CONFIG_MULTIPLEX_BY_BITMASKING == 1)
#define iomuxGPIO4Reg 					*((volatile uint32_t*)(IO_MUX_BASE | IO_MUX_GPIOn_REG_OFFS(4)))
#define iomuxGPIO5Reg 					*((volatile uint32_t*)(IO_MUX_BASE | IO_MUX_GPIOn_REG_OFFS(5)))
#define IO_MUX_GPIOn_FUN_DRV 			10
#define IO_MUX_GPIOn_MCU_SEL 			12
#else
struct IoMuxGpioReg {
	uint32_t ioMuxGpioMcuOe : 1;    // bit0
	uint32_t ioMuxGpioSlpSel : 1;   // bit1
	uint32_t ioMuxGpioMcuWpd : 1;   // bit2
	uint32_t ioMuxGpioMcuWpu : 1;   // bit3
	uint32_t ioMuxGpioMcuIe : 1;    // bit4
	uint32_t reserved0 : 2;         // bit5..6
	uint32_t ioMuxGpioFunWpd : 1;   // bit7
	uint32_t ioMuxGpioFunWpu : 1;   // bit8
	uint32_t ioMuxGpioFunIe : 1;    // bit9
	uint32_t ioMuxGpioFunDrv : 2;   // bit10..11
	uint32_t ioMuxGpioMcuSel : 3;   // bit12..14
	uint32_t ioMuxGpioFilterEn : 1; // bit15
	uint32_t : 16;                  // bit16..31
};

volatile struct IoMuxGpioReg* pIoMuxGpio4Reg = (volatile struct IoMuxGpioReg*)(IO_MUX_BASE | IO_MUX_GPIOn_REG_OFFS(4));
#define ioMuxGpio5Reg 					(*(volatile struct IoMuxGpioReg*)(IO_MUX_BASE | IO_MUX_GPIOn_REG_OFFS(5)))
#endif

#define LED1_GPIO						GPIO_NUM_4
#define LED2_GPIO						GPIO_NUM_5

// ***** implementation *****
void app_main() {
	printf("switch_led demo\n");

	// set pin multiplexing
	#if (CONFIG_MULTIPLEX_BY_BITMASKING == 1)
	iomuxGPIO4Reg &= ~(0x7 << IO_MUX_GPIOn_MCU_SEL);
	iomuxGPIO4Reg |= (IO_MUX_PIN_FUNC_GPIO << IO_MUX_GPIOn_MCU_SEL);
	iomuxGPIO4Reg &= ~(0x3 << IO_MUX_GPIOn_FUN_DRV);
	iomuxGPIO4Reg |= (IO_MUX_PIN_DRV_40mA << IO_MUX_GPIOn_FUN_DRV);

	iomuxGPIO5Reg &= ~(0x7 << IO_MUX_GPIOn_MCU_SEL);
	iomuxGPIO5Reg |= (IO_MUX_PIN_FUNC_GPIO << IO_MUX_GPIOn_MCU_SEL);
	iomuxGPIO5Reg &= ~(0x3 << IO_MUX_GPIOn_FUN_DRV);
	iomuxGPIO5Reg |= (IO_MUX_PIN_DRV_40mA << IO_MUX_GPIOn_FUN_DRV);

	/*
	 * or written with higher density and only one read/write access:
	 * uint32_t reg = (iomuxGPIO4Reg & ~((0x7 << IO_MUX_GPIOn_MCU_SEL) | (0x3 << IO_MUX_GPIOn_FUN_DRV)));
	 * iomuxGPIO4Reg = (reg | ((IO_MUX_PIN_FUNC_GPIO << IO_MUX_GPIOn_MCU_SEL) | (IO_MUX_PIN_DRV_40mA << IO_MUX_GPIOn_FUN_DRV)));
	 *
	 * simpler, without the use of the temporary reg value:
	 * iomuxGPIO5Reg &= ~((0x7 << IO_MUX_GPIOn_MCU_SEL) | (0x3 << IO_MUX_GPIOn_FUN_DRV));
	 * iomuxGPIO5Reg |= ((IO_MUX_PIN_FUNC_GPIO << IO_MUX_GPIOn_MCU_SEL) | (IO_MUX_PIN_DRV_40mA << IO_MUX_GPIOn_FUN_DRV));
	 */
	#else
    pIoMuxGpio4Reg->ioMuxGpioMcuSel = IO_MUX_PIN_FUNC_GPIO;
    pIoMuxGpio4Reg->ioMuxGpioFunDrv = IO_MUX_PIN_DRV_40mA;

    ioMuxGpio5Reg.ioMuxGpioMcuSel = IO_MUX_PIN_FUNC_GPIO;
    ioMuxGpio5Reg.ioMuxGpioFunDrv = IO_MUX_PIN_DRV_40mA;
	#endif

    // enable the GPIO output drivers
	gpioEnableReg |= (1 << GPIO_NUM_4) | (1 << GPIO_NUM_5);

	// "eternal" blink
	while (true) {
		gpioOutReg ^= (1 << LED1_GPIO) | (1 << LED2_GPIO);
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}
