/*
 * Example of the book "Embedded Systems mit RISC-V", dpunkt.verlag
 * Author: Patrick Ritschel
 *
 * see https://ritschel.at/kapitel-4-mikrocontroller-beispiel-rng_test/
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

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define RNG_BASE					0x60026000
#define RNG_DATA_REG_OFFS			0xB0

#define LOWPOWER_MGR_BASE			0x60008000
#define RTC_CNTL_CLK_CONF_REG		0x70

#define RTC_CNTL_DIG_FOSC_EN_BIT	10
#define RTC_CNTL_DIG_FOSC_EN		(0x1 << RTC_CNTL_DIG_FOSC_EN_BIT)
#define RTC_CNTL_FOSC_DFREQ_BIT		17
#define MASK_8BIT					((1 << 8) - 1)
#define RTC_CNTL_FOSC_DFREQ_MASK	(MASK_8BIT << RTC_CNTL_FOSC_DFREQ_BIT)

#define SQUARE(x)            		((x) * (x))

volatile uint32_t* pRngDataReg = (volatile uint32_t*)(RNG_BASE | RNG_DATA_REG_OFFS);
#define rtcCntlClkConfReg 			*((volatile uint32_t*)(LOWPOWER_MGR_BASE | RTC_CNTL_CLK_CONF_REG))

// first index is degree of freedom, second is 1-alpha: 0.900, 0.950, 0.975, 0.990, 0.995, 0.999
#define CHI_SQUARED_INDEX_DEGREE_OF_FREEDOM_1		0
#define CHI_SQUARED_INDEX_DEGREE_OF_FREEDOM_2		1
#define CHI_SQUARED_INDEX_DEGREE_OF_FREEDOM_3		2
#define CHI_SQUARED_INDEX_DEGREE_OF_FREEDOM_4		3
#define CHI_SQUARED_INDEX_DEGREE_OF_FREEDOM_5		4
#define CHI_SQUARED_INDEX_DEGREE_OF_FREEDOM_6		5
#define CHI_SQUARED_INDEX_DEGREE_OF_FREEDOM_7		6
#define CHI_SQUARED_INDEX_DEGREE_OF_FREEDOM_8		7
#define CHI_SQUARED_INDEX_DEGREE_OF_FREEDOM_9		8
#define CHI_SQUARED_INDEX_DEGREE_OF_FREEDOM_10		9
#define CHI_SQUARED_INDEX_DEGREE_OF_FREEDOM_11		10
#define CHI_SQUARED_INDEX_DEGREE_OF_FREEDOM_12		11
#define CHI_SQUARED_INDEX_DEGREE_OF_FREEDOM_13		12
#define CHI_SQUARED_INDEX_DEGREE_OF_FREEDOM_14		13
#define CHI_SQUARED_INDEX_DEGREE_OF_FREEDOM_15		14
#define CHI_SQUARED_INDEX_DEGREE_OF_FREEDOM_16		15
#define CHI_SQUARED_INDEX_DEGREE_OF_FREEDOM_17		16
#define CHI_SQUARED_INDEX_DEGREE_OF_FREEDOM_18		17
#define CHI_SQUARED_INDEX_DEGREE_OF_FREEDOM_19		18
#define CHI_SQUARED_INDEX_DEGREE_OF_FREEDOM_20		19
#define CHI_SQUARED_INDEX_DEGREE_OF_FREEDOM_21		20
#define CHI_SQUARED_INDEX_DEGREE_OF_FREEDOM_22		21
#define CHI_SQUARED_INDEX_DEGREE_OF_FREEDOM_23		22
#define CHI_SQUARED_INDEX_DEGREE_OF_FREEDOM_24		23
#define CHI_SQUARED_INDEX_DEGREE_OF_FREEDOM_25		24
#define CHI_SQUARED_INDEX_DEGREE_OF_FREEDOM_26		25
#define CHI_SQUARED_INDEX_DEGREE_OF_FREEDOM_27		26
#define CHI_SQUARED_INDEX_DEGREE_OF_FREEDOM_28		27
#define CHI_SQUARED_INDEX_DEGREE_OF_FREEDOM_29		28
#define CHI_SQUARED_INDEX_DEGREE_OF_FREEDOM_30		29
#define CHI_SQUARED_INDEX_DEGREE_OF_FREEDOM_40		30
#define CHI_SQUARED_INDEX_DEGREE_OF_FREEDOM_50		31
#define CHI_SQUARED_INDEX_DEGREE_OF_FREEDOM_60		32
#define CHI_SQUARED_INDEX_DEGREE_OF_FREEDOM_70		33
#define CHI_SQUARED_INDEX_DEGREE_OF_FREEDOM_80		34
#define CHI_SQUARED_INDEX_DEGREE_OF_FREEDOM_90		35
#define CHI_SQUARED_INDEX_DEGREE_OF_FREEDOM_100		36
#define CHI_SQUARED_INDEX_DEGREE_OF_FREEDOM_200		37
#define CHI_SQUARED_INDEX_DEGREE_OF_FREEDOM_300		38
#define CHI_SQUARED_INDEX_DEGREE_OF_FREEDOM_400		39
#define CHI_SQUARED_INDEX_DEGREE_OF_FREEDOM_500		40

enum ChiSquaredAlpha {
	ChiSquaredAlpha_10_percent,
	ChiSquaredAlpha_5_percent,
	ChiSquaredAlpha_2_5_percent,
	ChiSquaredAlpha_1_percent,
	ChiSquaredAlpha_0_5_percent,
	ChiSquaredAlpha_0_1_percent
};

float chi_squared_quantiles[41][6] = {
		{   2.710,	 3.840,	  5.020,   6.630,	7.880,	10.830 }, // 1
		{   4.610,	 5.990,	  7.380,   9.210,  10.600,	13.820 }, // 2
		{   6.250,	 7.810,	  9.350,  11.340,  12.840,	16.270 }, // 3
		{   7.780,	 9.490,	 11.140,  13.280,  14.860,	18.470 }, // 4
		{   9.240,	11.070,	 12.830,  15.090,  16.750,	20.520 }, // 5
		{  10.640,	12.590,	 14.450,  16.810,  18.550,	22.460 }, // 6
		{  12.020,	14.070,  16.010,  18.480,  20.280,	24.320 }, // 7
		{  13.360,	15.510,	 17.530,  20.090,  21.950,	26.120 }, // 8
		{  14.680,	16.920,	 19.020,  21.670,  23.590,	27.880 }, // 9
		{  15.990,	18.310,	 20.480,  23.210,  25.190,	29.590 }, // 10
		{  17.280,	19.680,	 21.920,  24.720,  26.760,	31.260 }, // 11
		{  18.550,	21.030,	 23.340,  26.220,  28.300,	32.910 }, // 12
		{  19.810,	22.360,	 24.740,  27.690,  29.820,	34.530 }, // 13
		{  21.060,	23.680,	 26.120,  29.140,  31.320,	36.120 }, // 14
		{  22.310,	25.000,	 27.490,  30.580,  32.800,	37.700 }, // 15
		{  23.540,	26.300,  28.850,  32.000,  34.270,	39.250 }, // 16
		{  24.770,	27.590,	 30.190,  33.410,  35.720,	40.790 }, // 17
		{  25.990,	28.870,	 31.530,  34.810,  37.160,	42.310 }, // 18
		{  27.200,	30.140,	 32.850,  36.190,  38.580,	43.820 }, // 19
		{  28.410,	31.410,	 34.170,  37.570,  40.000,	45.310 }, // 20
		{  29.620,	32.670,	 35.480,  38.930,  41.400,	46.800 }, // 21
		{  30.810,	33.920,	 36.780,  40.290,  42.800,	48.270 }, // 22
		{  32.010,	35.170,	 38.080,  41.640,  44.180,	49.730 }, // 23
		{  33.200,	36.420,	 39.360,  42.980,  45.560,	51.180 }, // 24
		{  34.380,	37.650,	 40.650,  44.310,  46.930,	52.620 }, // 25
		{  35.560,	38.890,	 41.920,  45.640,  48.290,	54.050 }, // 26
		{  36.740,	40.110,	 43.190,  46.960,  49.640,	55.480 }, // 27
		{  37.920,	41.340,	 44.460,  48.280,  50.990,	56.890 }, // 28
		{  39.090,	42.560,	 45.720,  49.590,  52.340,	58.300 }, // 29
		{  40.260,  43.770,	 46.980,  50.890,  53.670,	59.700 }, // 30
		{  51.810,	55.760,	 59.340,  63.690,  66.770,	73.400 }, // 40
		{  63.170,	67.500,	 71.420,  76.150,  79.490,	86.660 }, // 50
		{  74.400,	79.080,	 83.300,  88.380,  91.950,	99.610 }, // 60
		{  85.530,	90.530,	 95.020, 100.430, 104.210, 112.320 }, // 70
		{  96.580, 101.880,	106.630, 112.330, 116.320, 124.840 }, // 80
		{ 107.570, 113.150,	118.140, 124.120, 128.300, 137.210 }, // 90
		{ 118.500, 124.340,	129.560, 135.810, 140.170, 149.450 }, // 100
		{ 226.020, 233.990,	241.060, 249.450, 255.260, 267.540 }, // 200
		{ 331.790, 341.400,	349.870, 359.910, 366.840, 381.430 }, // 300
		{ 436.650, 447.630, 457.310, 468.720, 476.610, 493.130 }, // 400
		{ 540.930, 553.130, 563.850, 576.490, 585.210, 603.450 }  // 500
};

typedef enum {
	Status_Ok,
	Status_Chi2failed,
	Status_OutOfMemory
} Status_t;

Status_t equalDistChi2(const uint32_t n[], uint32_t m, uint32_t n0, uint32_t chi2);
float chiSquared(uint32_t degreesOfFreedom, enum ChiSquaredAlpha alpha);

// ***** implementation *****
inline uint32_t nextRand() {
	return *pRngDataReg;
}

inline void switchOnRtc20MClk() {
   rtcCntlClkConfReg |= RTC_CNTL_DIG_FOSC_EN;
}

inline void switchOffRtc20MClk() {
   rtcCntlClkConfReg &= ~RTC_CNTL_DIG_FOSC_EN;
}

void setRtcFOscDFreq(uint8_t dfreq) {
	uint32_t reg = rtcCntlClkConfReg;
	reg &= ~RTC_CNTL_FOSC_DFREQ_MASK;
	reg |= (dfreq << RTC_CNTL_FOSC_DFREQ_BIT);
	rtcCntlClkConfReg = reg;
}

Status_t equalDistChi2(const uint32_t n[], uint32_t m, uint32_t n0, uint32_t chi2) {
    uint64_t squaresum = 0;
    for (int i = 0; i < m; i += 1) {
        squaresum += SQUARE(n[i] - n0);
        printf("%d:%ld, ", i, n[i]);
    }
    printf("\n");
    uint64_t x2 = squaresum / n0;
    if (x2 <= chi2) {
    	return Status_Ok;
    } else {
    	return Status_Chi2failed;
    }
}

float chiSquared(uint32_t degreesOfFreedom, enum ChiSquaredAlpha alpha) {
	if (degreesOfFreedom >= 500) {
		return chi_squared_quantiles[CHI_SQUARED_INDEX_DEGREE_OF_FREEDOM_500][alpha];
	}
	if (degreesOfFreedom >= 100) {
		return chi_squared_quantiles[CHI_SQUARED_INDEX_DEGREE_OF_FREEDOM_100 + (degreesOfFreedom / 100) - 1][alpha];
	}
	if (degreesOfFreedom >= 30) {
		return chi_squared_quantiles[CHI_SQUARED_INDEX_DEGREE_OF_FREEDOM_30 + (degreesOfFreedom / 10) - 3][alpha];
	}
	if (degreesOfFreedom >= 1) {
		return chi_squared_quantiles[CHI_SQUARED_INDEX_DEGREE_OF_FREEDOM_1 + degreesOfFreedom - 1][alpha];
	}
	return 0.0;
}

Status_t testRNG(uint32_t observations, uint32_t m) {
	uint32_t* n = calloc(m, sizeof(uint32_t));
	if (n == NULL) {
		return Status_OutOfMemory;
	}
	for (int i = 0; i < observations; i += 1) {
		#if CONFIG_CALCULATE_MOD_M == 1
		n[nextRand() % m] += 1;
		#elif CONFIG_CALCULATE_BYTE_MOD_M == 1
		n[(nextRand() & 0xFF) % m] += 1;
		#else
		n[(nextRand() & 0x0F) % m] += 1;
		#endif
	}
	Status_t status = equalDistChi2(n, m, (observations / m), chiSquared(m - 1, ChiSquaredAlpha_10_percent));
	free(n);
	n = NULL;
	return status;
}

void app_main() {
	printf("Chi-Square RNG Test\n");
	switchOffRtc20MClk();
	switchOnRtc20MClk();
	setRtcFOscDFreq(177);
	printf("Clock switched on.\n");
	switch (testRNG(CONFIG_OBSERVATIONS, CONFIG_M)) {
		case Status_Ok:
			printf("Distribution ok\n");
			break;
		case Status_Chi2failed:
			printf("Distribution not ok\n");
			break;
		case Status_OutOfMemory:
			printf("Error: Out of memory!\n");
			break;
	}

	// "eternal" loop
    while (true) {
    	vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
