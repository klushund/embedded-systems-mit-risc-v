/*
 * Example of the book "Embedded Systems mit RISC-V und ESP32-C3", dpunkt.verlag
 * Author: Patrick Ritschel
 *
 * see https://ritschel.at/buchprojekt-pulsoximeter/
 *
 * This module contains the implementation of the pulse detection as described in the book.
 * It also uses Maxim's algorithm (a copy is in components/3rdparty) for pulse and SPO2-detection,
 * taken from https://www.analog.com/en/design-center/reference-designs/maxrefdes117.html
 * Aee also https://github.com/sparkfun/SparkFun_MAX3010x_Sensor_Library/blob/master/src/MAX30105.cpp
 * See also https://morf.lv/implementing-pulse-oximeter-using-max30100
 *
 * The code of this project is in the Public Domain (or CC0 licensed, at your option).
 * Unless required by applicable law or agreed to in writing, this
 * software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied.
 */

#include <stdbool.h>
#include "filter.h"
#include "driver/i2c.h"
#include "driver/gpio.h"

#define PULSEOXI_SAMPLINGRATE_Hz						100

#define PULSEOXI_HEARTBEATSPO2DETECTION_BUFFERSECONDS	5
#define PULSEOXI_HEARTBEATSPO2DETECTION_BUFFERLENGTH	(PULSEOXI_SAMPLINGRATE_Hz * PULSEOXI_HEARTBEATSPO2DETECTION_BUFFERSECONDS)

#define PULSEOXI_MAX_FFT_SIZE							512

#define PULSEOXI_NOPULSE								-1
#define PULSEOXI_IRREGULARPULSE							-2

typedef void (*pulseoxi_measurementCallback)(float pressure, float spo2);

#define PULSEOXI_MODE_CALLBACKONEVERYSAMPLE				0x01
#define PULSEOXI_MODE_FASTHEARTBEATDETECTION			0x02
#define PULSEOXI_MODE_HEARTBEATSPO2DETECTION			0x04
#define PULSEOXI_MODE_PRECISEFFTHEARTBEATDETECTION		0x08

struct PulseOxiSettings_t {
	i2c_port_t i2cPort;
	gpio_num_t gpioIRQ;
	uint8_t modes;
	bool debugMode;
	pulseoxi_measurementCallback measurementCallback;
};

// used for PULSEOXI_MODE_CALLBACKONEVERYSAMPLE and PULSEOXI_MODE_FASTHEARTBEATDETECTION
struct PulseOxiSingleSampleState_t {
	// data buffer
	float irLEDValue;
	float redLEDValue;

	Filter* pDCIRFilter;
	Filter* pDCRedFilter;
	Filter* pLPFilter;
	Filter* pMeanFilter;
};

struct PulseOxiFastHeartbeatDetectionState_t {
	// algorithm state
	bool pulseDetected;
	int16_t currentPulse_bpm;
};

struct PulseOxiHeartbeatSpO2DetectionState_t {
	uint32_t* irBuffer;
	uint32_t* redBuffer;
	int32_t currentSpO2Value;
	int8_t spo2Valid;
	int32_t currentHeartrate;
	int8_t hrValid;
	uint16_t bufferOffset;
};

struct PulseOxiPreciseFFTHeartbeatDetectionState_t {
	float* fftValues;
	uint32_t fftValuesOffset;
};

struct PulseOxiState_t {
	struct PulseOxiSingleSampleState_t singleSampleState;
	struct PulseOxiFastHeartbeatDetectionState_t fastHeartbeatDetectionState;
	struct PulseOxiHeartbeatSpO2DetectionState_t heartbeatSpO2DetectionState;
	struct PulseOxiPreciseFFTHeartbeatDetectionState_t preciseFFTState;
};


void pulseoxi_init(struct PulseOxiSettings_t* pSettings);
void pulseoxi_start();
const struct Max3010xDevice_t* pulseoxi_getMax3010xDevice();
const struct PulseOxiState_t* pulseoxi_getState();
void pulseoxi_resetPulseDetected(void);
void pulseoxi_update(void);
