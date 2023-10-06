/*
 * Example of the book "Embedded Systems mit RISC-V und ESP32-C3", dpunkt.verlag
 * Author: Patrick Ritschel
 *
 * see https://ritschel.at/buchprojekt-pulsoximeter/
 *
 * This module contains the BLE HR (heart rate) device implementation.
 *
 * The code of this project is in the Public Domain (or CC0 licensed, at your option).
 * Unless required by applicable law or agreed to in writing, this
 * software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied.
 */


#ifndef MAIN_BLEHRDEVICE_H_
#define MAIN_BLEHRDEVICE_H_

#include "esp_err.h"

#define GATT_HRS_UUID                           			0x180D
#define GATT_HRS_MEASUREMENT_UUID               			0x2A37
#define GATT_HRS_BODY_SENSOR_LOC_UUID           			0x2A38
#define GATT_DEVICE_INFO_UUID                   			0x180A
#define GATT_MANUFACTURER_NAME_UUID             			0x2A29
#define GATT_MODEL_NUMBER_UUID                  			0x2A24

#define GATT_SENSOR_LOCATION_OTHER 							0
#define GATT_SENSOR_LOCATION_CHEST 							1
#define GATT_SENSOR_LOCATION_WRIST 							2
#define GATT_SENSOR_LOCATION_FINGER 						3
#define GATT_SENSOR_LOCATION_HAND 							4
#define GATT_SENSOR_LOCATION_EAR_LOBE 						5
#define GATT_SENSOR_LOCATION_FOOT 							6

#define GATT_SENSOR_HEARTBEAT_FLAG_VALUEFORMAT_UINT8		0x00
#define GATT_SENSOR_HEARTBEAT_FLAG_VALUEFORMAT_UINT16		0x01
#define GATT_SENSOR_HEARTBEAT_FLAG_SENSORCONTACT_POOR		0x00
#define GATT_SENSOR_HEARTBEAT_FLAG_SENSORCONTACT_GOOD		0x02
#define GATT_SENSOR_HEARTBEAT_FLAG_SENSORCONTACT_NOTSUPPD	0x00
#define GATT_SENSOR_HEARTBEAT_FLAG_SENSORCONTACT_SUPPORTED	0x04
#define GATT_SENSOR_HEARTBEAT_FLAG_ENERGYEXPD_NOTPRESENT	0x00
#define GATT_SENSOR_HEARTBEAT_FLAG_ENERGYEXPD_PRESENT		0x08
#define GATT_SENSOR_HEARTBEAT_FLAG_RRINTERVAL_NOTPRESENT	0x00
#define GATT_SENSOR_HEARTBEAT_FLAG_RRINTERVAL_PRESENT		0x10
esp_err_t blehrdevice_init(void);
void blehrdevice_start(void);
void blehrdevice_notifyHeartbeat(int16_t pulse_bpm);

#endif /* MAIN_BLEHRDEVICE_H_ */
