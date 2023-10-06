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

#ifndef MAIN_GRAPHICS_H_
#define MAIN_GRAPHICS_H_

#include <stdint.h>
#include "driver/i2c.h"

void graphics_startUpdate(void);
void graphics_finishUpdate(void);
esp_err_t graphics_init(i2c_port_t i2c, uint8_t displayWidth, uint8_t displayHeight, uint8_t xOffs, bool flipVertical, bool synchronousUpdate);
uint8_t graphics_getDisplayWidth();
uint8_t graphics_getDisplayHeight();
void graphics_clearScreen(void);
void graphics_clearRegion(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);
void graphics_scrollLine(void);
void graphics_setImage(int x1, int y1, uint8_t width, uint8_t height, const uint8_t* image);
void graphics_setCursor(int x, int y);
void graphics_setPixel(int x, int y);
void graphics_clearPixel(int x, int y);
void graphics_invertPixel(int x, int y);
void graphics_setPixelValue(int x, int y, uint8_t pixel);
void graphics_writeChars(char* text, uint8_t textlen);
void graphics_writeString(char* text);
void graphics_println(char* text);
void graphics_drawLine(int x1, int y1, int x2, int y2, int pattern);

#endif /* MAIN_GRAPHICS_H_ */
