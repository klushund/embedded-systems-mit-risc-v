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
#include "fonts.h"

#if (CONFIG_GRAPHICS_USE_FONT_SKETCHFLOW_PRINT == 1)
#include "../fonts/Sketchflow_Print.ttf.inc"
#endif

#if (CONFIG_GRAPHICS_USE_FONT_STENCIL == 1)
#include "../fonts/STENCIL.TTF.inc"
#endif

#if (CONFIG_GRAPHICS_USE_FONT_TREBUCHET_MS == 1)
#include "../fonts/trebuc.ttf.inc"
#endif

static enum Graphics_Font gActiveFont =
#if (CONFIG_GRAPHICS_DEFAULT_FONT_SKETCHFLOW_PRINT == 1)
		Graphics_Font_SketchFlow_Print_16;
#elif (CONFIG_GRAPHICS_DEFAULT_FONT_STENCIL == 1)
		Graphics_Font_Stencil_16;
#elif (CONFIG_GRAPHICS_DEFAULT_FONT_TREBUCHET_MS == 1)
		Graphics_Font_Trebuchet_MS_16;
#endif

static const struct FontInfo gFontInfos[] = {
	#if (CONFIG_GRAPHICS_USE_FONT_SKETCHFLOW_PRINT == 1)
		{ .characterCount = SSD1306_FONT_SketchFlow_Print_16_CHARACTERCOUNT,
		  .characters = gFont_SketchFlow_Print_16_Characters,
		  .offsets = gFont_SketchFlow_Print_16_CharacterOffsets,
		  .height = 16
		},
	#else
		{ 0 },
	#endif
	#if (CONFIG_GRAPHICS_USE_FONT_STENCIL == 1)
		{ .characterCount = SSD1306_FONT_Stencil_16_CHARACTERCOUNT,
		  .characters = gFont_Stencil_16_Characters,
		  .offsets = gFont_Stencil_16_CharacterOffsets,
		  .height = 16
		},
	#else
		{ 0 },
	#endif
	#if (CONFIG_GRAPHICS_USE_FONT_TREBUCHET_MS == 1)
		{ .characterCount = SSD1306_FONT_Trebuchet_MS_16_CHARACTERCOUNT,
		  .characters = gFont_Trebuchet_MS_16_Characters,
		  .offsets = gFont_Trebuchet_MS_16_CharacterOffsets,
		  .height = 16
		},
	#endif
};

void fonts_setActiveFont(enum Graphics_Font font) {
	gActiveFont = font;
}

uint16_t fonts_getCharacterWidth(char c) {
	return (gFontInfos[gActiveFont].offsets[(int)(c+1)] - gFontInfos[gActiveFont].offsets[(int)c]) / 2;
}

const uint8_t* fonts_getCharacter(char c) {
	if (fonts_getCharacterWidth(c) == 0) {
		return &(gFontInfos[gActiveFont].characters[0]);
	}
	return &(gFontInfos[gActiveFont].characters[gFontInfos[gActiveFont].offsets[(int)c]]);
}

uint8_t fonts_getCharacterHeight() {
	return gFontInfos[gActiveFont].height;
}
