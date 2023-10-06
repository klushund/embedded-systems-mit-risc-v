/*
 * Example of the book "Embedded Systems mit RISC-V", dpunkt.verlag
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
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "led_strip.h"
#include "firfilter.h"
#include "iirfilter.h"

static const char* TAG = "SERVOCONTROL";

static led_strip_handle_t gLedStrip;

void app_main(void) {
	// Use ADC1 in oneshot mode
	adc_oneshot_unit_handle_t adcHandle;
	adc_oneshot_unit_init_cfg_t adcConfig = {
	    .unit_id = ADC_UNIT_1,
	    .ulp_mode = ADC_ULP_MODE_DISABLE,
	};
	ESP_ERROR_CHECK(adc_oneshot_new_unit(&adcConfig, &adcHandle));

    // ADC1 config; channel 2 maps on GPIO2
	adc_oneshot_chan_cfg_t channelConfig = {
	    .bitwidth = ADC_BITWIDTH_12,
	    .atten = ADC_ATTEN_DB_11,
	};
	ESP_ERROR_CHECK(adc_oneshot_config_channel(adcHandle, ADC_CHANNEL_2, &channelConfig));

	ESP_LOGI(TAG, "calibration scheme version is %s", "Curve Fitting");
	adc_cali_curve_fitting_config_t calConfig = {
	    .unit_id = ADC_UNIT_1,
	    .atten = ADC_ATTEN_DB_11,
	    .bitwidth = ADC_BITWIDTH_12,
	};
	adc_cali_handle_t calHandle;
	ESP_ERROR_CHECK(adc_cali_create_scheme_curve_fitting(&calConfig, &calHandle));

    // Initialize WS2812 LED Strip with 25 pixels
    led_strip_config_t stripConfig = {
        .strip_gpio_num = 8,
        .max_leds = 25
    };
    led_strip_rmt_config_t rmtConfig = {
        .resolution_hz = 10 * 1000 * 1000 // 10MHz
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&stripConfig, &rmtConfig, &gLedStrip));
    led_strip_clear(gLedStrip);

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
    	int rawValue, voltage_mV;
    	adc_oneshot_read(adcHandle, ADC_CHANNEL_2, &rawValue);
    	adc_cali_raw_to_voltage(calHandle, rawValue, &voltage_mV);

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
		led_strip_set_pixel(gLedStrip, 0, brightness, brightness, brightness);
		led_strip_refresh(gLedStrip);

		int32_t newduty = (1024/20) + percent * (1024/2000.0);// [1 ms, 2 ms]
		if (abs(newduty - duty) > 1) {
			duty = newduty;
			ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty));
			ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0));
		}

		printf("{P1|RESIST|0,255,0|%ld}\n", resist_ohm);
		printf("{P2|BRIGHT|200,0,0|%d}\n", brightness);

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
