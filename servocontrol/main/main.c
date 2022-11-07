/*
 * Example of the book "Embedded Systems mit RISC-V", dpunkt-Verlag
 * Author: Patrick Ritschel
 *
 * see https://ritschel.at/kapitel-8-5-4-pulsweitenmodulation-pwm-applikation-servocontrol/
 *
 * Based on ADC-Example from ESP-IDF.
 *
 * The code of this project is in the Public Domain (or CC0 licensed, at your option).
 * Unless required by applicable law or agreed to in writing, this
 * software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include "driver/ledc.h"
#include "esp_adc_cal.h"
#include "led_strip.h"
#include "firfilter.h"
#include "iirfilter.h"

static const char* TAG = "SERVOCONTROL";

static led_strip_t* pStrip_a;

void app_main(void) {
    // ADC1 config; ADC1_CHANNEL_2 maps on GPIO2
    ESP_ERROR_CHECK(adc1_config_width(ADC_WIDTH_BIT_12));
    ESP_ERROR_CHECK(adc1_config_channel_atten(ADC1_CHANNEL_2, ADC_ATTEN_DB_11));

    // Load ADC1 calibration data
    esp_adc_cal_characteristics_t adc1_calib;
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK) {
		esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11,
		ADC_WIDTH_BIT_12, 0, &adc1_calib);
    } else {
    	ESP_LOGE(TAG, "Could not calibrate ADC1, restarting now!");
        esp_restart();
    }

    // Initialize LED
    pStrip_a = led_strip_init(0, GPIO_NUM_8, 1);
    pStrip_a->clear(pStrip_a, 50);

    // Initialize Servo PWM
    ledc_timer_config_t ledc_timer = {
    	.speed_mode       = LEDC_LOW_SPEED_MODE,
    	.timer_num        = LEDC_TIMER_0,
    	.duty_resolution  = LEDC_TIMER_10_BIT,
    	.freq_hz          = 50,  // Set output frequency at 50 Hz; 20 ms period length
    	.clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));
    ledc_channel_config_t ledc_channel = {
    	.speed_mode     = LEDC_LOW_SPEED_MODE,
    	.channel        = LEDC_CHANNEL_0,
    	.timer_sel      = LEDC_TIMER_0,
    	.intr_type      = LEDC_INTR_DISABLE,
    	.gpio_num       = GPIO_NUM_3,
    	.duty           = 512, // Set duty to 50% // mid position
    	.hpoint         = 0,
		.flags.output_invert = 1
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

    // Create Filters
    float b2[2] = { 0.5, 0.5 };
    Filter* pFIRFilter_order2 = firfilter_create(b2, 2);
    float b10[10] = { 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1 };
    Filter* pFIRFilter_order10 = firfilter_create(b10, 10);
    Filter* pIIRFilter = iirfilter_create(0.4142, 0.0, 0.2929, 0.2929, 0.0);

    int32_t duty = 50;
    while (1) {
    	// read and transform ADC value -> voltage -> resistance
    	int rawValue = adc1_get_raw(ADC1_CHANNEL_2);
    	uint32_t voltage_mV = esp_adc_cal_raw_to_voltage(rawValue, &adc1_calib);

		float firValue_2 = filter_filterValue(pFIRFilter_order2, voltage_mV);
		float firValue_10 = filter_filterValue(pFIRFilter_order10, voltage_mV);
		float iirValue = filter_filterValue(pIIRFilter, voltage_mV);
		printf("{P0|RAW|0,255,0|%d|FIR2|0,0,255|%0.1f|FIR10|200,0,0|%0.1f|IIR|130,130,0|%0.1f}\n", voltage_mV, firValue_2, firValue_10, iirValue);

		#if (CONFIG_USE_FIR2_FILTER == 1)
		int32_t resist_ohm = (firValue_2 * 10000) / 2500;
		#elif (CONFIG_USE_FIR10_FILTER == 1)
		int32_t resist_ohm = (firValue_10 * 10000) / 2500;
		#else
		int32_t resist_ohm = (iirValue * 10000) / 2500;
		#endif

		// light LED according to resistance; scale to [0%,100%]
		uint8_t percent = MIN((resist_ohm * 100) / 10000, 100);
		uint8_t brightness = MIN(powf(percent / 6.25, 2), 255); // scale to [0..16], then to [0..256]
		pStrip_a->set_pixel(pStrip_a, 0, brightness, brightness, brightness);
		pStrip_a->refresh(pStrip_a, 100);

		uint32_t newduty = (1024/20) + percent * (1024/2000.0);// (uint32_t)(brightness * (1023/100.0));
		if (abs(newduty - duty) > 1) {
			duty = newduty;
			ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty));
			ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0));
		}

		printf("{P1|RESIST|0,255,0|%d}\n", resist_ohm);
		printf("{P2|BRIGHT|200,0,0|%d}\n", brightness);

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
