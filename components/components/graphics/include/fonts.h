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

#ifndef COMPONENTS_GRAPHICS_FONTS_H_
#define COMPONENTS_GRAPHICS_FONTS_H_

#include <stdint.h>
#include "sdkconfig.h"

enum Graphics_Font {
	#if (CONFIG_GRAPHICS_USE_FONT_SKETCHFLOW_PRINT == 1)
	Graphics_Font_SketchFlow_Print_16 = 0,
	#endif
	#if (CONFIG_GRAPHICS_USE_FONT_STENCIL == 1)
	Graphics_Font_Stencil_16 = 1,
	#endif
	#if (CONFIG_GRAPHICS_USE_FONT_TREBUCHET_MS == 1)
	Graphics_Font_Trebuchet_MS_16 = 2
	#endif
};

struct FontInfo {
	uint16_t characterCount;
	const uint8_t* characters;
	const uint16_t* offsets;
	uint8_t height;
};

void fonts_setActiveFont(enum Graphics_Font font);
uint16_t fonts_getCharacterWidth(char c);
const uint8_t* fonts_getCharacter(char c);
uint8_t fonts_getCharacterHeight(void);

#endif /* COMPONENTS_GRAPHICS_FONTS_H_ */
