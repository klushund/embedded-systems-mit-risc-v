/*
 * Example of the book "Embedded Systems mit RISC-V und ESP32-C3", dpunkt.verlag
 * Author: Patrick Ritschel
 *
 * see https://ritschel.at/buchprojekt-pulsoximeter/
 *
 * This module sets up a minimal webserver with a page showing the current pulse.
 *
 * The code of this project is in the Public Domain (or CC0 licensed, at your option).
 * Unless required by applicable law or agreed to in writing, this
 * software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied.
 */

#include "webserver.h"
#include "pulseoxi.h"

static esp_err_t getPulseHandler(httpd_req_t *req);

// ***** implementation *****
esp_err_t getPulseHandler(httpd_req_t *req) {
    char buf[256];
    snprintf(buf, 256, "<!DOCTYPE html>"
    		"<html>"
    		"<body>"
    		"<h1>Poxi</h1>"
    		"<h2>Pulse</h2>"
    		"<p><b>%d bpm</b></p>"
    		"</body>"
    		"</html>", pulseoxi_getState()->fastHeartbeatDetectionState.currentPulse_bpm);
    httpd_resp_send(req, buf, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

httpd_uri_t uriGetPulse = {
    .uri      = "/pulse",
    .method   = HTTP_GET,
    .handler  = getPulseHandler,
    .user_ctx = NULL
};

httpd_handle_t webserver_start(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;

    // Start the httpd server
    if (httpd_start(&server, &config) == ESP_OK) {
        // Register URI handlers
        httpd_register_uri_handler(server, &uriGetPulse);
    }
    return server;
}

void webserver_stop(httpd_handle_t server) {
    if (server) {
        httpd_stop(server);
    }
}
