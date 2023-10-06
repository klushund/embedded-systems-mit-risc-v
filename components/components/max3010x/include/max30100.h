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

#ifndef COMPONENTS_PULSEOXI_INCLUDE_MAX30100_H_
#define COMPONENTS_PULSEOXI_INCLUDE_MAX30100_H_

#include "max3010x.h"

// registers
#define MAX30100_REG_FIFOWRPTR							0x02
#define MAX30100_REG_FIFOOVFCTR							0x03
#define MAX30100_REG_FIFORDPTR							0x04
#define MAX30100_REG_FIFODATA							0x05
#define MAX30100_REG_MODECONFIG							0x06
#define MAX30100_REG_SPO2CONFIG							0x07
#define MAX30100_REG_LEDCONFIG							0x09

// masks
#define MAX30100_MODECONFIG_RESET						(1 << 6)

#define MAX30100_SPO2CONFIG_HIRESEN						(1 << 6)

#define MAX30100_SAMPLINGRATE_50Hz						0x00
#define MAX30100_SAMPLINGRATE_100Hz						0x01
#define MAX30100_SAMPLINGRATE_167Hz						0x02
#define MAX30100_SAMPLINGRATE_200Hz						0x03
#define MAX30100_SAMPLINGRATE_400Hz						0x04
#define MAX30100_SAMPLINGRATE_600Hz						0x05
#define MAX30100_SAMPLINGRATE_800Hz						0x06
#define MAX30100_SAMPLINGRATE_1000Hz					0x07

#define MAX30100_LEDPULSEWIDTH_200us_ADC13				0x00
#define MAX30100_LEDPULSEWIDTH_400us_ADC14				0x01
#define MAX30100_LEDPULSEWIDTH_800us_ADC15				0x02
#define MAX30100_LEDPULSEWIDTH_1600us_ADC16				0x03

#define MAX30100_LEDCURRENT_0mA							0x00
#define MAX30100_LEDCURRENT_4_4mA						0x01
#define MAX30100_LEDCURRENT_7_6mA						0x02
#define MAX30100_LEDCURRENT_11mA						0x03
#define MAX30100_LEDCURRENT_14_2mA						0x04
#define MAX30100_LEDCURRENT_17_4mA						0x05
#define MAX30100_LEDCURRENT_20_8mA						0x06
#define MAX30100_LEDCURRENT_24mA						0x07
#define MAX30100_LEDCURRENT_27_1mA						0x08
#define MAX30100_LEDCURRENT_30_6mA						0x09
#define MAX30100_LEDCURRENT_33_8mA						0x0A
#define MAX30100_LEDCURRENT_37mA						0x0B
#define MAX30100_LEDCURRENT_40_2mA						0x0C
#define MAX30100_LEDCURRENT_43_6mA						0x0D
#define MAX30100_LEDCURRENT_46_8mA						0x0E
#define MAX30100_LEDCURRENT_50mA						0x0F


void max30100_initCallbacks(struct Max3010xDevice_t* pDevice);

#endif /* COMPONENTS_PULSEOXI_INCLUDE_MAX30100_H_ */
