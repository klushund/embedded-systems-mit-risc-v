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

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include "esp_dsp.h"
#include "esp_timer.h"
#include "max3010x.h"
#include "algorithm.h"
#include "pulseoxi.h"
#include "dcfilter.h"
#include "lpfilter.h"
#include "meanfilter.h"

#define TAG								"pulseoxi"

#define LEDRAWBUFFERSIZE				32

#define PULSEOXI_TASK_STACKSIZE			4096
#define PULSEOXI_TASK_PRIORITY			3

// filter parameters
#define DCFILTER_ALPHA 					0.95f
#define LPFILTER_ALPHA					0.8f

/* FFT Calculations
 * Abtastrate fs = 100 Hz
 * Blocklänge BL = 256
 * Bandbreite fn = fs / 2 = 50 Hz
 * Messdauer D = BL / fs = 2,56 s
 * Frequenzauflösung df = fs / BL = 0,39 Hz
 */
#define FFT_SAMPLINGRATE			((float)PULSEOXI_SAMPLINGRATE_Hz)
#define FFT_FREQ_RESOLUTION			(FFT_SAMPLINGRATE / PULSEOXI_MAX_FFT_SIZE)

static struct PulseOxiSettings_t gSettings;

static struct PulseOxiState_t gState = { 0 };
static TaskHandle_t gPulseoxiTaskHandle = NULL;

static void pulseoxiTaskMainFunc(void * pvParameters);
static void dataAvailableCallback(void);
static void detectPulse(int pulseValue);


// ***** implementation *****
void pulseoxi_init(struct PulseOxiSettings_t* pSettings) {
	assert(pSettings->measurementCallback != NULL);
	gSettings = *pSettings;
	max3010x_init(gSettings.i2cPort, gSettings.gpioIRQ, dataAvailableCallback);
	// initialize states for the used modes
	if (gSettings.modes & (PULSEOXI_MODE_CALLBACKONEVERYSAMPLE | PULSEOXI_MODE_FASTHEARTBEATDETECTION))  {
		gState.singleSampleState.pDCIRFilter = dcfilter_create(DCFILTER_ALPHA);
		assert(gState.singleSampleState.pDCIRFilter != NULL);
		gState.singleSampleState.pDCRedFilter = dcfilter_create(DCFILTER_ALPHA);
		assert(gState.singleSampleState.pDCRedFilter != NULL);
		gState.singleSampleState.pLPFilter = lpfilter_create(LPFILTER_ALPHA);
		assert(gState.singleSampleState.pLPFilter != NULL);
		gState.singleSampleState.pMeanFilter = meanfilter_create(16);
		assert(gState.singleSampleState.pMeanFilter != NULL);
	}
	if (gSettings.modes & PULSEOXI_MODE_HEARTBEATSPO2DETECTION) {
		gState.heartbeatSpO2DetectionState.irBuffer = malloc(PULSEOXI_HEARTBEATSPO2DETECTION_BUFFERLENGTH * sizeof(uint32_t));
		assert(gState.heartbeatSpO2DetectionState.irBuffer != NULL);
		gState.heartbeatSpO2DetectionState.redBuffer = malloc(PULSEOXI_HEARTBEATSPO2DETECTION_BUFFERLENGTH * sizeof(uint32_t));
		assert(gState.heartbeatSpO2DetectionState.redBuffer != NULL);
		gState.heartbeatSpO2DetectionState.bufferOffset = 0;
		gState.heartbeatSpO2DetectionState.hrValid = false;
		gState.heartbeatSpO2DetectionState.spo2Valid = false;
	}
	if (gSettings.modes & PULSEOXI_MODE_PRECISEFFTHEARTBEATDETECTION) {
		gState.preciseFFTState.fftValuesOffset = 0;
		gState.preciseFFTState.fftValues = heap_caps_aligned_alloc(4, sizeof(float) * PULSEOXI_MAX_FFT_SIZE * 2, MALLOC_CAP_DEFAULT);
		assert(gState.preciseFFTState.fftValues != NULL);
		ESP_ERROR_CHECK(dsps_fft2r_init_fc32(NULL, PULSEOXI_MAX_FFT_SIZE));
	}
}

void pulseoxi_start() {
	// create task for data fetching and processing
	xTaskCreate(pulseoxiTaskMainFunc, "PULSEOXI", PULSEOXI_TASK_STACKSIZE, NULL, PULSEOXI_TASK_PRIORITY, &gPulseoxiTaskHandle);
	configASSERT(gPulseoxiTaskHandle);
}

const struct Max3010xDevice_t* pulseoxi_getMax3010xDevice() {
	return max3010x_getDevice();
}

const struct PulseOxiState_t* pulseoxi_getState() {
	return &gState;
}

void pulseoxi_resetPulseDetected() {
	gState.fastHeartbeatDetectionState.pulseDetected = false;
}

void pulseoxiTaskMainFunc(void * pvParameters) {
	uint32_t irLEDRawValues[LEDRAWBUFFERSIZE];
	uint32_t redLEDRawValues[LEDRAWBUFFERSIZE];

	max3010x_softReset();
	max3010x_setup(MAX3010X_MODE_SPO2_HR, PULSEOXI_SAMPLINGRATE_Hz);

	while (true) {
		if (gSettings.gpioIRQ != GPIO_NUM_NC) {
			ulTaskNotifyTake(pdTRUE, portMAX_DELAY); // wait for signaled interrupt
		}
		uint8_t cnt = max3010x_readFIFO(irLEDRawValues, redLEDRawValues, LEDRAWBUFFERSIZE);
		for (uint8_t i = 0; i < cnt; i += 1) {
			float irValue = (float)irLEDRawValues[i];
			float redValue = (float)redLEDRawValues[i];
			float delta = irValue - redValue;
			//These are special packets for FlexiPlot plotting tool
			if (gSettings.debugMode) {
				printf("{P0|IR|0,0,255|%.1f|RED|255,0,0|%.1f}\n", irValue, redValue);
			}

			if (gSettings.modes & (PULSEOXI_MODE_CALLBACKONEVERYSAMPLE | PULSEOXI_MODE_FASTHEARTBEATDETECTION)) {
				// apply filtering
				irValue = filter_filterValue(gState.singleSampleState.pDCIRFilter, irValue);
				redValue = filter_filterValue(gState.singleSampleState.pDCRedFilter, redValue);
				irValue = filter_filterValue(gState.singleSampleState.pLPFilter, irValue);
				irValue = filter_filterValue(gState.singleSampleState.pMeanFilter, irValue);

				if (gSettings.debugMode) {
					printf("{P1|IR|255,0,255|%.1f|BEAT|0,0,255|%.1f}\n", irValue, redValue);
				}

				if (gSettings.modes & PULSEOXI_MODE_CALLBACKONEVERYSAMPLE) {
					gSettings.measurementCallback(irValue, delta);
				}

				if (gSettings.modes & PULSEOXI_MODE_FASTHEARTBEATDETECTION) {
					detectPulse(irValue);
				}
			}

			if (gSettings.modes & PULSEOXI_MODE_HEARTBEATSPO2DETECTION) {
				gState.heartbeatSpO2DetectionState.irBuffer[gState.heartbeatSpO2DetectionState.bufferOffset] = irValue;
				gState.heartbeatSpO2DetectionState.redBuffer[gState.heartbeatSpO2DetectionState.bufferOffset] = redValue;

				gState.heartbeatSpO2DetectionState.bufferOffset += 1;
				if (gState.heartbeatSpO2DetectionState.bufferOffset == PULSEOXI_HEARTBEATSPO2DETECTION_BUFFERLENGTH) {
					maxim_heart_rate_and_oxygen_saturation(
							gState.heartbeatSpO2DetectionState.irBuffer, PULSEOXI_HEARTBEATSPO2DETECTION_BUFFERLENGTH, gState.heartbeatSpO2DetectionState.redBuffer,
							&(gState.heartbeatSpO2DetectionState.currentSpO2Value), &(gState.heartbeatSpO2DetectionState.spo2Valid),
							&(gState.heartbeatSpO2DetectionState.currentHeartrate), &(gState.heartbeatSpO2DetectionState.hrValid));
					printf("SpO2=%ld, valid=%d, heartrate=%ld, valid=%d\n", gState.heartbeatSpO2DetectionState.currentSpO2Value, gState.heartbeatSpO2DetectionState.spo2Valid,
							gState.heartbeatSpO2DetectionState.currentHeartrate, gState.heartbeatSpO2DetectionState.hrValid);

					// dumping the first 100 sets of samples in the memory and shift the last 400 sets of samples to the top
					for (int i=100; i<500; i++) {
						gState.heartbeatSpO2DetectionState.redBuffer[i-100] = gState.heartbeatSpO2DetectionState.redBuffer[i];
						gState.heartbeatSpO2DetectionState.irBuffer[i-100] = gState.heartbeatSpO2DetectionState.irBuffer[i];
					}
					gState.heartbeatSpO2DetectionState.bufferOffset = (PULSEOXI_HEARTBEATSPO2DETECTION_BUFFERSECONDS - 1) * PULSEOXI_SAMPLINGRATE_Hz;
				}
			}

			if (gSettings.modes & PULSEOXI_MODE_PRECISEFFTHEARTBEATDETECTION) {
				// take the sample value as complex vector (with real part only)
				gState.preciseFFTState.fftValues[gState.preciseFFTState.fftValuesOffset++] = irValue;
				gState.preciseFFTState.fftValues[gState.preciseFFTState.fftValuesOffset++] = 0;
				if (gState.preciseFFTState.fftValuesOffset == PULSEOXI_MAX_FFT_SIZE * 2) {
					dsps_fft2r_fc32(gState.preciseFFTState.fftValues, PULSEOXI_MAX_FFT_SIZE);
				    // Bit reverse
				    dsps_bit_rev_fc32(gState.preciseFFTState.fftValues, PULSEOXI_MAX_FFT_SIZE);
				    // Convert one complex vector to two complex vectors
				    dsps_cplx2reC_fc32(gState.preciseFFTState.fftValues, PULSEOXI_MAX_FFT_SIZE);

				    float max = -1000;
				    int idxmax = -1;
				    for (int i = 0; i < PULSEOXI_MAX_FFT_SIZE / 2; i++) {
				    	gState.preciseFFTState.fftValues[i] = 10 * log10f((gState.preciseFFTState.fftValues[i * 2 + 0] * gState.preciseFFTState.fftValues[i * 2 + 0] + gState.preciseFFTState.fftValues[i * 2 + 1] * gState.preciseFFTState.fftValues[i * 2 + 1]) / PULSEOXI_MAX_FFT_SIZE);
				    	if (gState.preciseFFTState.fftValues[i] > max) {
				    		max = gState.preciseFFTState.fftValues[i];
				    		idxmax = i;
				    	}
				    }
				    printf("FFT max bin: %f at %d\n", max, idxmax);
				    printf("FFT max freq: %f\n", (idxmax * FFT_FREQ_RESOLUTION * 60));
				    if (gSettings.debugMode) {
				    	// Show power spectrum in 64x10 window from -100 to 0 dB from 0..N/4 samples
				    	dsps_view(gState.preciseFFTState.fftValues, PULSEOXI_MAX_FFT_SIZE / 2, 64, 10, -40, 40, '|');
				    }
				    gState.preciseFFTState.fftValuesOffset = 0;
				}
			}
		}
		usleep(10000); // 10ms
	}
}

void dataAvailableCallback() {
	if (gPulseoxiTaskHandle != NULL) {
		BaseType_t higherPriorityTaskWoken = pdFALSE;
		vTaskNotifyGiveFromISR(gPulseoxiTaskHandle, &higherPriorityTaskWoken);
	}
}

void detectPulse(int pulse) {
	static int pulseValue = 0;
	static uint8_t cnt = 0;
	pulseValue += (int)pulse / 10;
	cnt += 1;
	if (cnt >= 3)  {
		pulseValue /= 3;

		static int localMax = 0;
		static int increases = 0;
		static int decreases = 0;
		static int lastBeatMax = 0;
		static uint32_t lastBeatTick = 0;
		static uint32_t localMaxTick = 0;

		if ((pulseValue > localMax) && (pulseValue > lastBeatMax)) {
			localMax = pulseValue;
			increases += 1;
			decreases = 0;
			localMaxTick = esp_timer_get_time();
		} else if ((pulseValue < localMax) || (pulseValue <= 0)) {
			decreases += 1;
			if (decreases == 2) {
				if ((increases >= 2) && (localMax > 2)) {
					gState.fastHeartbeatDetectionState.pulseDetected = true;
					lastBeatMax = localMax / 4;
					localMax = 0;
					increases = 0;
					decreases = 0;
					uint32_t delta = localMaxTick - lastBeatTick;
					gState.fastHeartbeatDetectionState.currentPulse_bpm = 60000 / (delta / 1000);
					lastBeatTick = localMaxTick;
				}
			} else if (decreases > 30) {
				increases = 0;
				lastBeatMax = lastBeatMax / 4;
				localMax = 0;
				gState.fastHeartbeatDetectionState.currentPulse_bpm = PULSEOXI_NOPULSE;
				decreases = 0;
			}
		}
		cnt = 0;
		pulseValue = 0;
	}
}
