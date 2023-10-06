/*
 * Example of the book "Embedded Systems mit RISC-V und ESP32-C3", dpunkt.verlag
 * Author: Patrick Ritschel
 *
 * see https://ritschel.at/buchprojekt-pulsoximeter/
 *
 * This module sets up WIFI in station mode with a static IP address.
 *
 * The code of this project is in the Public Domain (or CC0 licensed, at your option).
 * Unless required by applicable law or agreed to in writing, this
 * software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied.
 */

#include "sdkconfig.h"

#ifndef CONFIG_USE_PROVISIONING

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "sdkconfig.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "staticwifi.h"

#define xUSE_SEMAPHORE_TO_WAIT_FOR_IP

static esp_ip4_addr_t gIPAddr;
#ifdef USE_SEMAPHORE_TO_WAIT_FOR_IP
static xSemaphoreHandle s_semph_get_ip_addrs;
#endif
static esp_netif_t *gpNetIF = NULL;

static const char *TAG = "staticwifi";

static void staticwifi_shutdown(void);
static esp_netif_t *wifi_start(void);
static void wifi_stop(void);
static void on_wifi_disconnect(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
static void on_got_ip(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);

esp_err_t staticwifi_init() {
	ESP_LOGI(TAG, "Setting up static WIFI");
	ESP_ERROR_CHECK(esp_netif_init());
	ESP_ERROR_CHECK(esp_event_loop_create_default());

#ifdef USE_SEMAPHORE_TO_WAIT_FOR_IP
    if (s_semph_get_ip_addrs != NULL) {
        return ESP_ERR_INVALID_STATE;
    }
#endif
    gpNetIF = wifi_start();
#ifdef USE_SEMAPHORE_TO_WAIT_FOR_IP
     s_semph_get_ip_addrs = xSemaphoreCreateCounting(1, 0);
#endif

    ESP_ERROR_CHECK(esp_register_shutdown_handler(&staticwifi_shutdown));
#ifdef USE_SEMAPHORE_TO_WAIT_FOR_IP
    ESP_LOGI(TAG, "Waiting for IP");
	xSemaphoreTake(s_semph_get_ip_addrs, portMAX_DELAY);
#endif
    return ESP_OK;
}

void staticwifi_shutdown() {
    wifi_stop();
}

esp_netif_t *wifi_start() {
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_netif_inherent_config_t esp_netif_config = ESP_NETIF_INHERENT_DEFAULT_WIFI_STA();
    esp_netif_t *netif = esp_netif_create_wifi(WIFI_IF_STA, &esp_netif_config);
    esp_wifi_set_default_wifi_sta_handlers();

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &on_wifi_disconnect, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &on_got_ip, NULL));

    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = CONFIG_WIFI_SSID,
            .password = CONFIG_WIFI_PASSWORD,
            .scan_method = WIFI_FAST_SCAN,
            .sort_method = WIFI_CONNECT_AP_BY_SIGNAL,
            .threshold.rssi = -127,
            .threshold.authmode = WIFI_AUTH_OPEN,
        },
    };
    ESP_LOGI(TAG, "Connecting to %s...", wifi_config.sta.ssid);
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    esp_wifi_connect();
    return netif;
}

static void wifi_stop(void) {
	if (gpNetIF == NULL) {
		return;
	}
    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &on_wifi_disconnect));
    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &on_got_ip));
    esp_err_t err = esp_wifi_stop();
    if (err == ESP_ERR_WIFI_NOT_INIT) {
        return;
    }
    ESP_ERROR_CHECK(err);
    ESP_ERROR_CHECK(esp_wifi_deinit());
    ESP_ERROR_CHECK(esp_wifi_clear_default_wifi_driver_and_handlers(gpNetIF));
    esp_netif_destroy(gpNetIF);
    gpNetIF = NULL;
}

void on_wifi_disconnect(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    ESP_LOGI(TAG, "Wi-Fi disconnected, trying to reconnect...");
    esp_err_t err = esp_wifi_connect();
    if (err == ESP_ERR_WIFI_NOT_STARTED) {
        return;
    }
    ESP_ERROR_CHECK(err);
}

static void on_got_ip(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    ESP_LOGI(TAG, "Got IPv4 event: Interface \"%s\" address: " IPSTR, esp_netif_get_desc(event->esp_netif), IP2STR(&event->ip_info.ip));
    memcpy(&gIPAddr, &event->ip_info.ip, sizeof(gIPAddr));
#ifdef USE_SEMAPHORE_TO_WAIT_FOR_IP
    xSemaphoreGive(s_semph_get_ip_addrs);
#endif
}

#endif // CONFIG_USE_PROVISIONING
