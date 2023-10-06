/*
 * Example of the book "Embedded Systems mit RISC-V und ESP32-C3", dpunkt.verlag
 * Author: Patrick Ritschel
 *
 * see https://ritschel.at/buchprojekt-pulsoximeter/
 *
 * The project contains a pulseoximeter that can send it's pulse data via WIFI/MQTT and BLE.
 * 
 * This module contains initialisation and main loop of the Poxi application.
 *
 * The code of this project is in the Public Domain (or CC0 licensed, at your option).
 * Unless required by applicable law or agreed to in writing, this
 * software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied.
 */

#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <nvs_flash.h>
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "led_strip.h"
#include "esp_timer.h"
#include "graphics.h"
#include "max3010x.h"
#include "pulseoxi.h"
#include "pushbtn.h"
#include "filter.h"
#include "iirfilter.h"
#if CONFIG_USE_PROVISIONING
#include "provisioning.h"
#else
#include "staticwifi.h"
#include "blehrdevice.h"
#endif
#if CONFIG_START_WEBSERVER
#include "webserver.h"
#endif
#include "sendpacket.h"
#include "mqtt.h"

#include "left_back.png.inc"
#include "right_back.png.inc"
#include "heart.png.inc"

#define I2C_INTERFACE				I2C_NUM_0
#define I2C_MASTER_BITRATE			400000

// internal prototypes
static esp_err_t initI2C(i2c_port_t i2c_num);
static void initLED(void);
static void drawBackground(void);
static void drawPulseOxi(void);
static void drawChart(float pressure, float spo2);

// module internal globals
static const char* TAG = "poxi";

static led_strip_handle_t gLedStrip;
static Filter* pPulseFilter = NULL;
static Filter* pOxiFilter = NULL;

// ***** implementation *****
esp_err_t initI2C(i2c_port_t i2c_num) {
	ESP_LOGI(TAG, "init I2C");
	i2c_config_t conf = {
		.mode = I2C_MODE_MASTER,
		.sda_io_num = GPIO_NUM_5,
		.scl_io_num = GPIO_NUM_6,
		.sda_pullup_en = GPIO_PULLUP_ENABLE,
		.scl_pullup_en = GPIO_PULLUP_ENABLE,
		.master.clk_speed = I2C_MASTER_BITRATE
	};
	i2c_param_config(i2c_num, &conf);
	esp_err_t res = i2c_driver_install(i2c_num, conf.mode, 0, 0, 0);
	if (res != ESP_OK) {
		ESP_LOGE(TAG, "i2c_driver_install() FAILED: %d!", res);
	} else {
		ESP_LOGI(TAG, "i2c_driver_install() OK");
	}
	return res;
}

void initLED() {
    // Initialize WS2812 LED Strip with 25 pixels
    led_strip_config_t stripConfig = {
        .strip_gpio_num = 8,
        .max_leds = 1
    };
    led_strip_rmt_config_t rmtConfig = {
        .resolution_hz = 10 * 1000 * 1000 // 10MHz
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&stripConfig, &rmtConfig, &gLedStrip));
    led_strip_clear(gLedStrip);
}

void drawBackground() {
	graphics_startUpdate();
	graphics_clearScreen();
	graphics_setImage(0, 0, LEFT_BACK_PNG_WIDTH, LEFT_BACK_PNG_HEIGHT, left_back_png);
	graphics_setImage(graphics_getDisplayWidth() - RIGHT_BACK_PNG_WIDTH - 1, 0, RIGHT_BACK_PNG_WIDTH, RIGHT_BACK_PNG_HEIGHT, right_back_png);
	graphics_finishUpdate();
}

void drawPulseOxi() {
	graphics_startUpdate();
	graphics_setCursor(10, 40);
	if (pulseoxi_getState()->fastHeartbeatDetectionState.currentPulse_bpm == PULSEOXI_NOPULSE) {
		graphics_writeString("NO FINGER");
	} else {
		char buf[16];
		sprintf(buf, "%d bpm", pulseoxi_getState()->fastHeartbeatDetectionState.currentPulse_bpm);
		graphics_writeString(buf);
	}
	graphics_finishUpdate();
}

void drawChart(float pressure, float spo2) {
	static int x = 4;
	static int cnt = 0;
	(void) spo2; // ignore SPO2 value
	cnt += 1;
	if (cnt >= 3)  {
		pressure /= 10; // scale
		int y = 24 - pressure;
		if (y < 0) y = 0;
		if (y > 31) y = 31;
		graphics_setPixel(x, y);
		if (y < 20) {
			led_strip_set_pixel(gLedStrip, 0, (24-y) / 4, 0, 0);
		} else {
			led_strip_set_pixel(gLedStrip, 0, 0, 0, 0);
		}

		x += 1;
		if (x > 96) {
			graphics_clearRegion(4, 0, 97, 64);
			for (int x = 4; x < 97; x += 4) {
				graphics_setPixel(x, 24); // horizontal line
			}
			x = 4;
		}
		cnt = 0;

		static uint8_t showPulse = 0;
		if (showPulse == 0) {
			if (pulseoxi_getState()->fastHeartbeatDetectionState.pulseDetected) {
				showPulse = 10;
				graphics_setImage(99, 4, HEART_PNG_WIDTH, HEART_PNG_HEIGHT, heart_png);
			}
		} else {
			if (showPulse == 1) {
				pulseoxi_resetPulseDetected();
				graphics_clearRegion(99, 0, 99 + HEART_PNG_WIDTH, 64);
			}
			showPulse -= 1;
		}
	}
}

void measurementCallback(float pulse, float oxi) {
	float filteredPulse = filter_filterValue(pPulseFilter, pulse);
	float filteredOxi = filter_filterValue(pOxiFilter, oxi);
	drawChart(filteredPulse, filteredOxi);
}

void app_main(void) {
    // Initialize NVS partition
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase()); // NVS partition was truncated and needs to be erased
        ESP_ERROR_CHECK(nvs_flash_init()); // Retry nvs_flash_init
    }
    ESP_ERROR_CHECK(ret);

    gpio_install_isr_service(0);

	// init i2c and SPI
    ESP_ERROR_CHECK(initI2C(I2C_INTERFACE));
	// init display
    ESP_ERROR_CHECK(graphics_init(I2C_INTERFACE, CONFIG_GRAPHICS_PIXELWIDTH, CONFIG_GRAPHICS_PIXELHEIGHT, 0, true, false));
	graphics_startUpdate();
	graphics_clearScreen();
	graphics_println("Poxi V1.0");
	graphics_println("=========");
	graphics_finishUpdate();

	char buf[32];
	struct PulseOxiSettings_t poSettings = {
			.i2cPort = I2C_INTERFACE,
			.gpioIRQ = GPIO_NUM_NC, // don't use IRQ
			.modes = (PULSEOXI_MODE_CALLBACKONEVERYSAMPLE | PULSEOXI_MODE_FASTHEARTBEATDETECTION | PULSEOXI_MODE_HEARTBEATSPO2DETECTION | PULSEOXI_MODE_PRECISEFFTHEARTBEATDETECTION),
			.debugMode = false,
			.measurementCallback = measurementCallback
	};
	pulseoxi_init(&poSettings);
	const struct Max3010xDevice_t* pMaxDevice = pulseoxi_getMax3010xDevice();
	if (pMaxDevice == NULL) {
		graphics_startUpdate();
		graphics_println("MAX3010x not found!");
		graphics_println("Cannot start!");
		graphics_println("Press Reset.");
		graphics_finishUpdate();
		while (true) {
			usleep(1000000);
		}
	}
	const char* deviceName = max3010x_getDeviceName(pMaxDevice);
	graphics_startUpdate();
	graphics_println("Found");
	sprintf(buf, "%s rev %d", deviceName, pMaxDevice->revision);
	graphics_println(buf);
	graphics_finishUpdate();
	usleep(1000000); // 1 s

	initLED();
	pushbtn_init();
	graphics_startUpdate();
	graphics_println("I/O started");
	graphics_finishUpdate();
	usleep(1000000); // 1 s

	#if CONFIG_USE_PROVISIONING
	provisioning_init();
	#else
	staticwifi_init();
	#endif
	#if CONFIG_START_WEBSERVER
	webserver_start();
	#endif

	mqtt_init();
	graphics_startUpdate();
	graphics_println("Net started");
	graphics_finishUpdate();

	MessageBufferHandle_t buttonMessageBuffer = xMessageBufferCreate(sizeof(struct ButtonEvent) * 2);
	configASSERT(buttonMessageBuffer);
	pushbtn_registerObserver(buttonMessageBuffer);

	#ifndef CONFIG_USE_PROVISIONING
	blehrdevice_init();
	blehrdevice_start();

	graphics_startUpdate();
	graphics_println("BLE started");
	graphics_finishUpdate();
	
	int64_t timePrev = 0; // for use in main loop
	#endif

	usleep(1000000); // 1 s

	pPulseFilter = iirfilter_create(0.4142, 0.0, 0.2929, 0.2929, 0.0);
	configASSERT(pPulseFilter != NULL);
	pOxiFilter = iirfilter_create(0.4142, 0.0, 0.2929, 0.2929, 0.0);
	configASSERT(pOxiFilter != NULL);

	graphics_startUpdate();
	graphics_println("System is up.");
	graphics_finishUpdate();
	usleep(3000000); // 1 s

	drawBackground();

	// start pulseoxi task
	pulseoxi_start();

	// main loop: check button, refresh LED, do network things
	while (1) {
    	// check button
		struct ButtonEvent buttonEvent;
		if (xMessageBufferReceive(buttonMessageBuffer, &buttonEvent, sizeof(struct ButtonEvent), 0) == sizeof(struct ButtonEvent)) {
			ESP_LOGI(TAG, "btn: %d\n", buttonEvent.buttonState);
    		char buf[64];
    		if (buttonEvent.buttonState == ButtonState_Pressed) {
    			if (pulseoxi_getState()->fastHeartbeatDetectionState.currentPulse_bpm == PULSEOXI_NOPULSE) {
    				sprintf(buf, "No Finger at: %lld ms\n", (buttonEvent.systemtime / 1000));
    			} else {
    				sprintf(buf, "Pulse %d at: %lld ms\n", pulseoxi_getState()->fastHeartbeatDetectionState.currentPulse_bpm, (buttonEvent.systemtime / 1000));
    			}
    			// send pulse data to a server
				#if CONFIG_TRANSPORT_UDP
    			sendpacket_sendUDP(CONFIG_IPV4_ADDR, CONFIG_PORT, (uint8_t*)buf, strlen(buf));
				#elif CONFIG_TRANSPORT_TCP
    			sprintf(buf, "{\"Pulse\": %d, \"Time\": %lld}", pulseoxi_getState()->fastHeartbeatDetectionState.currentPulse_bpm, (buttonEvent.systemtime / 1000));
    			sendpacket_sendTCP(CONFIG_IPV4_ADDR, CONFIG_PORT, (uint8_t*)buf, strlen(buf));
				#endif
    			sendpacket_sendMQTT((uint8_t*)buf, strlen(buf));
    		}
    	}

		drawPulseOxi();
		led_strip_refresh(gLedStrip);
		#ifndef CONFIG_USE_PROVISIONING
		int64_t timeNow = esp_timer_get_time();
		if (timeNow - timePrev >= 1000000) { // maximum once a second
			blehrdevice_notifyHeartbeat(pulseoxi_getState()->fastHeartbeatDetectionState.currentPulse_bpm);
			timePrev = timeNow;
		}
		#endif

		usleep(10000); // 10 ms
	}
}
