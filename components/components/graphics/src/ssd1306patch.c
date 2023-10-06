/*
 * Part of the book "Embedded Systems mit RISC-V", dpunkt.verlag
 * Author: Patrick Ritschel
 *
 * see https://ritschel.at/buch-embedded-systems-auf-den-punkt-gebracht/
 *
 * The code of this project is in the Public Domain (or CC0 licensed, at your option).
 * Unless required by applicable law or agreed to in writing, this
 * software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied.
 */

#include <sys/cdefs.h>
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "ssd1306patch.h"

typedef struct {
    esp_lcd_panel_t base;
    esp_lcd_panel_io_handle_t io;
    int reset_gpio_num;
    bool reset_level;
    int x_gap;
    int y_gap;
    unsigned int bits_per_pixel;
} ssd1306_panel_t;


esp_err_t ssd1306patch_sendCommand(esp_lcd_panel_t *panel, uint8_t command, uint8_t data) {
    // access the panel directly to set the display precharge (brighter)
    ssd1306_panel_t *ssd1306 = __containerof(panel, ssd1306_panel_t, base);
    esp_lcd_panel_io_handle_t io = ssd1306->io;
    return esp_lcd_panel_io_tx_param(io, command, (uint8_t[]) {
        data
    }, 1);
}
