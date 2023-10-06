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
#include "esp_lcd_panel_interface.h"

#define SSD1306_COMMAND_SETCONTRAST			0x81
#define SSD1306_COMMAND_SETPRECHARGE		0xD9

esp_err_t ssd1306patch_sendCommand(esp_lcd_panel_t *panel, uint8_t command, uint8_t data);
