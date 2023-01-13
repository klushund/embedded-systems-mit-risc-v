/*
 * Example of the book "Embedded Systems mit RISC-V", dpunkt.verlag
 * Author: Patrick Ritschel
 *
 * see https://ritschel.at/kapitel-7-4-spi-schnittstelle-beispiel-sdcard_demo/
 *
 * Based on the basic Espressif IDF-project.
 *
 * The code of this project is in the Public Domain (or CC0 licensed, at your option).
 * Unless required by applicable law or agreed to in writing, this
 * software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied.
 */
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"

void app_main() {
	printf("sdcard_demo\n");
	spi_bus_config_t bus_cfg = {
		.mosi_io_num = 5,
		.miso_io_num = 6,
		.sclk_io_num = 4,
		.quadwp_io_num = -1,
		.quadhd_io_num = -1,
		.max_transfer_sz = 0 // DMA default
	};
	ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &bus_cfg, SPI_DMA_CH_AUTO));

	sdmmc_host_t host = SDSPI_HOST_DEFAULT();
	host.slot = SPI2_HOST;
	sdspi_device_config_t slotCfg = SDSPI_DEVICE_CONFIG_DEFAULT();
	slotCfg.gpio_cs = 7;
	slotCfg.host_id = host.slot;
	esp_vfs_fat_sdmmc_mount_config_t mountCfg = {
		.format_if_mount_failed = false,
		.max_files = 5,
		.allocation_unit_size = 16 * 1024
	};
	sdmmc_card_t* pSDCard; // the pointer will be set by-ref

	ESP_ERROR_CHECK(esp_vfs_fat_sdspi_mount("/sdcard", &host, &slotCfg, &mountCfg, &pSDCard));

	FILE* file = fopen("/sdcard/foo.txt", "r");
	if (file != NULL) {
		char line[64];
		fgets(line, sizeof(line), file);
		fclose(file);
	}

	esp_vfs_fat_sdcard_unmount("/sdcard", pSDCard);
	spi_bus_free(SPI2_HOST);
}
