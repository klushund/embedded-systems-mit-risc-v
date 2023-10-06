/*
 * Example of the book "Embedded Systems mit RISC-V", dpunkt.verlag
 * Author: Patrick Ritschel
 *
 * see https://ritschel.at/buch-embedded-systems-auf-den-punkt-gebracht/
 *
 * The code of this project is in the Public Domain (or CC0 licensed, at your option).
 * Unless required by applicable law or agreed to in writing, this
 * software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied.
 */

#ifndef COMPONENTS_PULSEOXI_INCLUDE_MAX30102_H_
#define COMPONENTS_PULSEOXI_INCLUDE_MAX30102_H_

#include "max3010x.h"

// registers
#define MAX30102_REG_INTERRUPTSTATUS1					0x00
#define MAX30102_REG_INTERRUPTSTATUS2					0x01
#define MAX30102_REG_INTERRUPTENABLE1					0x02
#define MAX30102_REG_INTERRUPTENABLE2					0x03
#define MAX30102_REG_FIFOWRPTR							0x04
#define MAX30102_REG_FIFOOVFCTR							0x05
#define MAX30102_REG_FIFORDPTR							0x06
#define MAX30102_REG_FIFODATA							0x07
#define MAX30102_REG_FIFOCONFIG							0x08
#define MAX30102_REG_MODECONFIG							0x09
#define MAX30102_REG_SPO2CONFIG							0x0A
#define MAX30102_REG_LEDPULSEAMP1						0x0C
#define MAX30102_REG_LEDPULSEAMP2						0x0D
#define MAX30102_REG_PILOTPA							0x10
#define MAX30102_REG_MULTILEDCTRL12						0x11
#define MAX30102_REG_MULTILEDCTRL34						0x12
#define MAX30102_REG_DIETEMP							0x1F
#define MAX30102_REG_DIETEMPFRAC						0x20
#define MAX30102_REG_DIETEMPCONFIG						0x21

// masks
#define MAX30102_INT1_PWRRDY							(1 << 0)
#define MAX30102_INT1_ALCOVF							(1 << 5)
#define MAX30102_INT1_PPGRDY							(1 << 6)
#define MAX30102_INT1_AFULL								(1 << 7)
#define MAX30102_INT2_DIETEMPDY							(1 << 1)

#define MAX30102_FIFOCONFIG_ROLLOVEREN					(1 << 4)
#define MAX30102_FIFOCONFIG_SMPAVENONE					(0 << 5)
#define MAX30102_FIFOCONFIG_SMPAVE2						(1 << 5)
#define MAX30102_FIFOCONFIG_SMPAVE4						(2 << 5)
#define MAX30102_FIFOCONFIG_SMPAVE8						(3 << 5)
#define MAX30102_FIFOCONFIG_SMPAVE16					(4 << 5)
#define MAX30102_FIFOCONFIG_SMPAVE32					(5 << 5)

#define MAX30102_MODECONFIG_SHDN						(1 << 7)
#define MAX30102_MODECONFIG_RESET						(1 << 6)
#define MAX30102_MODECONFIG_HEARTRATEMODE				(2)
#define MAX30102_MODECONFIG_SPO2MODE					(3)
#define MAX30102_MODECONFIG_MULTILEDMODE				(7)

#define MAX30102_SPO2CONFIG_LEDPW_69us_ADC15bit			(0)
#define MAX30102_SPO2CONFIG_LEDPW_118us_ADC16bit		(1)
#define MAX30102_SPO2CONFIG_LEDPW_215us_ADC17bit		(2)
#define MAX30102_SPO2CONFIG_LEDPW_411us_ADC18bit		(3)

#define MAX30102_SPO2CONFIG_SAMPLERATE_50				(0 << 2)
#define MAX30102_SPO2CONFIG_SAMPLERATE_100				(1 << 2)
#define MAX30102_SPO2CONFIG_SAMPLERATE_200				(2 << 2)
#define MAX30102_SPO2CONFIG_SAMPLERATE_400				(3 << 2)
#define MAX30102_SPO2CONFIG_SAMPLERATE_800				(4 << 2)
#define MAX30102_SPO2CONFIG_SAMPLERATE_1000				(5 << 2)
#define MAX30102_SPO2CONFIG_SAMPLERATE_1600				(6 << 2)
#define MAX30102_SPO2CONFIG_SAMPLERATE_3200				(7 << 2)

#define MAX30102_SPO2CONFIG_ADCRGE_00					(0 << 5) // 2048 nA
#define MAX30102_SPO2CONFIG_ADCRGE_01					(1 << 5) // 4096 nA
#define MAX30102_SPO2CONFIG_ADCRGE_10					(2 << 5) // 8192 nA
#define MAX30102_SPO2CONFIG_ADCRGE_11					(3 << 5) // 16384 nA

#define MAX30102_LEDCURRENT_uA_TO_REGVAL(current)		(current / 200)

#define MAX30102_FIFOSIZE								32


void max30102_initCallbacks(struct Max3010xDevice_t* pDevice);

#endif /* COMPONENTS_PULSEOXI_INCLUDE_MAX30102_H_ */
