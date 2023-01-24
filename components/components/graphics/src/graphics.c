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
#include <stdlib.h>
#include <string.h>

#include "sdkconfig.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "ssd1306patch.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "fonts.h"
#include "graphics.h"

#if (CONFIG_GRAPHICS_BITSPERPIXEL != 1)
	#error "The graphics module currently supports only monochrome displays!"
#endif

#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

#define DISPLAY_TASK_STACKSIZE		2048

// module internal globals
static esp_lcd_panel_handle_t gDisplayHandle = NULL;

static uint8_t gDisplayWidth;
static uint8_t gDisplayHeight;
static bool gSynchronousUpdate;
static uint8_t gXOffs;

static uint8_t* gDisplayBuffer;
static uint8_t* gTransferBuffer;
static uint16_t gBufferLength;
static int gDirtyX1;
static int gDirtyY1;
static int gDirtyX2;
static int gDirtyY2;

static int gCursorX;
static int gCursorY;

static SemaphoreHandle_t gMutex = NULL;
TaskHandle_t gDisplayTaskHandle = NULL;

// internal prototypes
static void displayTaskMainFunc(void * pvParameters);
static void createDisplayTask(void);
static void adaptDirty(int x1, int y1, int x2, int y2);
static void sendDirtyDisplayBuffer(void);
static uint8_t getImagePixel(int x, int y, uint8_t width, uint8_t height, const uint8_t* image);

// ***** implementation *****
void graphics_startUpdate() {
	xSemaphoreTakeRecursive(gMutex, portMAX_DELAY);
}

void graphics_finishUpdate() {
	if (gSynchronousUpdate) {
		sendDirtyDisplayBuffer();
	}
	xSemaphoreGiveRecursive(gMutex);
	xTaskNotifyGive(gDisplayTaskHandle);
}

esp_lcd_panel_handle_t initDisplay(esp_lcd_i2c_bus_handle_t displayInterface, uint32_t i2cAddr) {
	esp_lcd_panel_io_handle_t io_handle = NULL;
	esp_lcd_panel_io_i2c_config_t io_config = {
			.dev_addr = i2cAddr,
			.lcd_cmd_bits = 8,
			.lcd_param_bits = 8,
			.control_phase_bytes = 1,
			.dc_bit_offset = 6,
			.flags.dc_low_on_data = 0
	};
	// Attach the LCD to the I2C bus
	ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c(displayInterface, &io_config, &io_handle));

	esp_lcd_panel_handle_t displayHandle = NULL;
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = -1,
        .bits_per_pixel = 1
    };
    // Initialize the LCD configuration
    ESP_ERROR_CHECK(esp_lcd_new_panel_ssd1306(io_handle, &panel_config, &displayHandle));

    // Reset the display
    ESP_ERROR_CHECK(esp_lcd_panel_reset(displayHandle));

    // Initialize LCD panel
    ESP_ERROR_CHECK(esp_lcd_panel_init(displayHandle));

    // set precharge and contrast to max -> brighter
    ssd1306patch_sendCommand(displayHandle, SSD1306_COMMAND_SETPRECHARGE, 0xF2);

    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(displayHandle, true));

    return displayHandle;
}

void graphics_init(i2c_port_t i2c, uint8_t displayWidth, uint8_t displayHeight, uint8_t xOffs, bool synchronousUpdate) {
	gDisplayHandle = initDisplay((esp_lcd_i2c_bus_handle_t)i2c, CONFIG_GRAPHICS_I2CADDR);
	gDisplayWidth = displayWidth;
	gDisplayHeight = displayHeight;
	gSynchronousUpdate = synchronousUpdate;
	gXOffs = xOffs;

	// Create semaphor for critical section
	gMutex = xSemaphoreCreateRecursiveMutex();
	assert(gMutex != NULL);
	if (!gSynchronousUpdate) {
		createDisplayTask();
	}

	// Allocate memory for the display buffers
	gBufferLength = (uint16_t)gDisplayWidth * (uint16_t)gDisplayHeight / (8 / CONFIG_GRAPHICS_BITSPERPIXEL);
	gDisplayBuffer = malloc(gBufferLength);
	assert(gDisplayBuffer != NULL);
	gTransferBuffer = heap_caps_malloc(gBufferLength, MALLOC_CAP_DMA);
	assert(gTransferBuffer != NULL);
	graphics_clearScreen();
}

uint8_t graphics_getDisplayWidth() {
	return gDisplayWidth;
}

uint8_t graphics_getDisplayHeight() {
	return gDisplayHeight;
}

void displayTaskMainFunc(void * pvParameters) {
	for( ;; ) {
		if (ulTaskNotifyTake(pdTRUE, portMAX_DELAY) == 1) {
			// send out the dirty display buffer
			sendDirtyDisplayBuffer();
		}
	}
}

void createDisplayTask() {
	static uint8_t ucParameterToPass; // must persist through the task's lifetime!

	xTaskCreate(displayTaskMainFunc, "DISPLAY", DISPLAY_TASK_STACKSIZE, &ucParameterToPass, 3, &gDisplayTaskHandle);
	configASSERT(gDisplayTaskHandle);
}

void graphics_clearScreen() {
	graphics_clearRegion(0, 0, gDisplayWidth, gDisplayHeight);
	gCursorY = 0;
	gCursorX = 0;
}

// x1 excl, x2 incl.
void graphics_clearRegion(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2) {
	uint8_t lineStart = y1 / 8;
	uint8_t lineEnd = (y2 % 8 == 0) ? y2 / 8 : y2 / 8 + 1;

	while (lineStart <= lineEnd) {
		memset(gDisplayBuffer + x1 + lineStart * gDisplayWidth, 0x00, x2 - x1);
		lineStart += 1;
	}

	adaptDirty(x1, y1, x2, y2);
}

// x1 incl, x2 excl
void adaptDirty(int x1, int y1, int x2, int y2) {
	// round lines
	y1 /= 8;
	y1 *= 8;
	y2 = (y2 % 8 == 0) ? y2 / 8 : y2 / 8 + 1;
	y2 *= 8;
	gDirtyX1 = MIN(gDirtyX1, x1);
	gDirtyY1 = MIN(gDirtyY1, y1);
	gDirtyX2 = MAX(gDirtyX2, x2);
	gDirtyY2 = MAX(gDirtyY2, y2);
}

void sendDirtyDisplayBuffer() {
	if (xSemaphoreTakeRecursive(gMutex, portMAX_DELAY) == pdTRUE) {
		// copy buffer and ranges to output
		memcpy(gTransferBuffer, gDisplayBuffer, gBufferLength);
		int transferX1 = 0;
		int transferX2 = gDisplayWidth;
		int transferY1 = gDirtyY1 / 8;
		int transferY2 = (gDirtyY2 % 8 == 0) ? (gDirtyY2 / 8) : (gDirtyY2 / 8 + 1);
		gDirtyX1 = gDisplayWidth;
		gDirtyX2 = 0;
		gDirtyY1 = gDisplayHeight;
		gDirtyY2 = 0;
		xSemaphoreGiveRecursive(gMutex);
		for (int y = transferY1; y < transferY2; y += 1) {
			esp_lcd_panel_draw_bitmap(gDisplayHandle, transferX1 + gXOffs, y * 8, transferX2 + gXOffs, (y+1) * 8, gTransferBuffer + y * gDisplayWidth + transferX1);
			vTaskDelay(pdMS_TO_TICKS(2));
		}
	}
}

void graphics_scrollLine() {
	int height = fonts_getCharacterHeight();
	int lineBytes = gDisplayWidth * height / 8;
	// move one line upwards
	memcpy(gDisplayBuffer, gDisplayBuffer + lineBytes, gBufferLength - lineBytes);
	// clear last line
	memset(gDisplayBuffer + gBufferLength - lineBytes, 0x00, lineBytes);
	adaptDirty(0, 0, gDisplayWidth, gDisplayHeight);
}

void graphics_setImage(int x1, int y1, uint8_t width, uint8_t height, const uint8_t* image) {
	int y2 = y1 + height;
	int x2 = MIN(gDisplayWidth, x1 + width);
	if ((y1 % 8 == 0) && (y2 % 8 == 0)) {
		uint8_t lineStart = y1 / 8;
		uint8_t lineEnd = (y2 % 8 == 0) ? y2 / 8 : y2 / 8 + 1;

		int i = 0;
		while (lineStart < lineEnd) {
			memcpy(gDisplayBuffer + x1 + lineStart * gDisplayWidth, image + i * width, x2 - x1);
			lineStart += 1;
			i += 1;
		}
	} else {
		for (uint8_t x = 0; x < width; x += 1) {
			for (uint8_t y = 0; y < height; y += 1) {
				graphics_setPixelValue(x1 + x, y1 + y, getImagePixel(x, y, width, height, image));
			}
		}
	}

	adaptDirty(x1, y1, x2, y2);
}

void graphics_setCursor(int x, int y) {
	gCursorX = x;
	gCursorY = y;
}

void graphics_setPixel(int x, int y) {
	if ((x < gDisplayWidth) && (y < gDisplayHeight)) {
		gDisplayBuffer[x + (y / 8) * gDisplayWidth] |= 1 << (y % 8);
		adaptDirty(x, y, x+1, y+1);
	}
}

void graphics_clearPixel(int x, int y) {
	if ((x < gDisplayWidth) && (y < gDisplayHeight)) {
		gDisplayBuffer[x + (y / 8) * gDisplayWidth] &= ~(1 << (y % 8));
		adaptDirty(x, y, x+1, y+1);
	}
}

void graphics_invertPixel(int x, int y) {
	if ((x < gDisplayWidth) && (y < gDisplayHeight)) {
		gDisplayBuffer[x + (y / 8) * gDisplayWidth] ^= 1 << (y % 8);
		adaptDirty(x, y, x+1, y+1);
	}
}

void graphics_setPixelValue(int x, int y, uint8_t pixel) {
	if ((x < gDisplayWidth) && (y < gDisplayHeight)) {
		if (pixel) {
			gDisplayBuffer[x + (y / 8) * gDisplayWidth] |= 1 << (y % 8);
		} else {
			gDisplayBuffer[x + (y / 8) * gDisplayWidth] &= ~(1 << (y % 8));
		}
		adaptDirty(x, y, x+1, y+1);
	}
}

uint8_t getImagePixel(int x, int y, uint8_t width, uint8_t height, const uint8_t* image) {
	return image[x + (y / 8) * width] & (1 << (y % 8));
}

void graphics_writeChars(char* text, uint8_t textlen) {
	int x1 = gCursorX;
	int y1 = gCursorY;
	int height = fonts_getCharacterHeight();
	while (textlen--) {
		if ((gCursorX >= gDisplayWidth) || (gCursorY >= gDisplayHeight)) {
			break;
		}
		int width = fonts_getCharacterWidth(*text);
		graphics_setImage(gCursorX, gCursorY, width, height, fonts_getCharacter(*text));
		text += 1;
		gCursorX += width;
	}
	gCursorY += height;
	adaptDirty(x1, y1, gCursorX+1, gCursorY);
	gCursorX = 0;
}

void graphics_writeString(char* text) {
	graphics_writeChars(text, strlen(text));
}

void graphics_println(char* text) {
	if (gCursorY >= gDisplayHeight) {
		graphics_scrollLine();
		gCursorY -= fonts_getCharacterHeight();
	}
	graphics_writeString(text);
}

void graphics_drawLine(int x1, int y1, int x2, int y2, int pattern) {
	if (abs(x2-x1) > abs(y2-y1)) {
		// draw "horizontal"
		// x1 must be <= x2 => change on demand
		if (x1 > x2) {
			int temp = y1;
			y1 = y2;
			y2 = temp;
			temp = x1;
			x1 = x2;
			x2 = temp;
		}
		float deltaY = (y2 - y1) / (float)(x2 - x1);
		float y = y1;
		int count = 0;
		while (x1 <= x2) {
			if (count == 0) {
				graphics_setPixel(x1, y);
			}
			count += 1;
			count %= pattern;
			y += deltaY;
			x1 += 1;
		}
	} else {
		// draw "vertical"
		// x1 must be <= x2 => change on demand
		if (y1 > y2) {
			int temp = y1;
			y1 = y2;
			y2 = temp;
			temp = x1;
			x1 = x2;
			x2 = temp;
		}
		float deltaX = (x2 - x1) / (float)(y2 - y1);
		float x = x1;
		int count = 0;
		while (y1 <= y2) {
			if (count == 0) {
				graphics_setPixel(x, y1);
			}
			count += 1;
			count %= pattern;
			x += deltaX;
			y1 += 1;
		}
	}
}
