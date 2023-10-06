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

#include "blehrdevice.h"
#include "host/ble_gap.h"
#include "esp_nimble_hci.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

// ----- prototypes -----
static void printAddr(const uint8_t* addr);
static int handleGAPEvent(struct ble_gap_event *event, void *arg);
static void enableAdvertising(void);
static void onSync(void);
static void onReset(int reason);
static void bleHostTaskMain(void *param);
static esp_err_t gattCharacterisicAccessHeartRate(uint16_t connHandle, uint16_t attrHandle, struct ble_gatt_access_ctxt* ctxt, void* arg);
static esp_err_t gattCharacteristicAccessDeviceInfo(uint16_t connHandle, uint16_t attrHandle, struct ble_gatt_access_ctxt* ctxt, void* arg);
static esp_err_t initGATTServer(void);

// ----- globals -----
static const char* tag = "blehrdevice";
static const char* gDeviceName = "poxi_0.9";
static const char *gManufacturerName = "Patrick Ritschel";
static const char *gModelNum = "Poxi HR Sensor demo";

static uint8_t gAddrType;
static uint16_t gConnectionHandle;
static bool gNotifyState;
static uint16_t gHeartRateValueHandle;

static const struct ble_gatt_svc_def gGATTServices[] = {
    {
        // Service: Heart-rate, according to Bluetooth Heart Rate Service Spec. V10.0
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(GATT_HRS_UUID),
        .characteristics = (struct ble_gatt_chr_def[])
        { {
                // Characteristic: Heart-rate measurement
                .uuid = BLE_UUID16_DECLARE(GATT_HRS_MEASUREMENT_UUID),
                .access_cb = gattCharacterisicAccessHeartRate,
                .val_handle = &gHeartRateValueHandle,
                .flags = BLE_GATT_CHR_F_NOTIFY,
            }, {
                // Characteristic: Body sensor location
                .uuid = BLE_UUID16_DECLARE(GATT_HRS_BODY_SENSOR_LOC_UUID),
                .access_cb = gattCharacterisicAccessHeartRate,
                .flags = BLE_GATT_CHR_F_READ,
            }, {
                0, /* No more characteristics in this service */
            },
        }
    },
    {
        /* Service: Device Information */
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(GATT_DEVICE_INFO_UUID),
        .characteristics = (struct ble_gatt_chr_def[])
        { {
                /* Characteristic: * Manufacturer name */
                .uuid = BLE_UUID16_DECLARE(GATT_MANUFACTURER_NAME_UUID),
                .access_cb = gattCharacteristicAccessDeviceInfo,
                .flags = BLE_GATT_CHR_F_READ,
            }, {
                /* Characteristic: Model number string */
                .uuid = BLE_UUID16_DECLARE(GATT_MODEL_NUMBER_UUID),
                .access_cb = gattCharacteristicAccessDeviceInfo,
                .flags = BLE_GATT_CHR_F_READ,
            }, {
                0, /* No more characteristics in this service */
            },
        }
    },
    {
        0, /* No more services */
    },
};

// ----- Implementation -----
void printAddr(const uint8_t* addr) {
    MODLOG_DFLT(INFO, "%02x:%02x:%02x:%02x:%02x:%02x",
                addr[5], addr[4], addr[3], addr[2], addr[1], addr[0]);
}

int handleGAPEvent(struct ble_gap_event *event, void *arg) {
    switch (event->type) {
		case BLE_GAP_EVENT_CONNECT:
			// A new connection was established or a connection attempt failed
			MODLOG_DFLT(INFO, "connection %s; status=%d\n", event->connect.status == 0 ? "established" : "failed", event->connect.status);
			if (event->connect.status != 0) {
				// Connection failed; resume advertising
				enableAdvertising();
			}
			gConnectionHandle = event->connect.conn_handle;
			break;

		case BLE_GAP_EVENT_DISCONNECT:
			MODLOG_DFLT(INFO, "disconnect; reason=%d\n", event->disconnect.reason);
			gConnectionHandle = 0;
			// Connection terminated; resume advertising
			enableAdvertising();
			break;

		case BLE_GAP_EVENT_ADV_COMPLETE:
			MODLOG_DFLT(INFO, "adv complete\n");
			enableAdvertising();
			break;

		case BLE_GAP_EVENT_SUBSCRIBE:
			MODLOG_DFLT(INFO, "subscribe event; cur_notify=%d\n value handle; val_handle=%d\n", event->subscribe.cur_notify, gHeartRateValueHandle);
			if (event->subscribe.attr_handle == gHeartRateValueHandle) {
				gNotifyState = event->subscribe.cur_notify;
			} else if (event->subscribe.attr_handle != gHeartRateValueHandle) {
				gNotifyState = event->subscribe.cur_notify;
			}
			ESP_LOGI("BLE_GAP_SUBSCRIBE_EVENT", "conn_handle from subscribe=%d", gConnectionHandle);
			break;

		case BLE_GAP_EVENT_MTU:
			MODLOG_DFLT(INFO, "mtu update event; conn_handle=%d mtu=%d\n", event->mtu.conn_handle, event->mtu.value);
			break;
	}
    return 0;
}

/* Enables advertising with parameters:
 *     o General discoverable mode
 *     o Undirected connectable mode
 */
void enableAdvertising() {
    struct ble_gap_adv_params advParams;
    struct ble_hs_adv_fields fields;
    int rc;

    /*  Set the advertisement data included in our advertisements:
     *     o Flags (indicates advertisement type and other general info)
     *     o Advertising tx power
     *     o Device name
     */
    memset(&fields, 0, sizeof(fields));
    /* Advertise two flags:
     *      o Discoverability in forthcoming advertisement (general)
     *      o BLE-only (BR/EDR unsupported)
     */
    fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;
    /* Indicate that the TX power level field should be included; have the
     * stack fill this value automatically.  This is done by assigning the
     * special value BLE_HS_ADV_TX_PWR_LVL_AUTO.
     */
    fields.tx_pwr_lvl_is_present = 1;
    fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;
    fields.name = (uint8_t *)gDeviceName;
    fields.name_len = strlen(gDeviceName);
    fields.name_is_complete = 1;
    rc = ble_gap_adv_set_fields(&fields);
    if (rc != 0) {
        MODLOG_DFLT(ERROR, "error setting advertisement data; rc=%d\n", rc);
        return;
    }

    // Begin advertising
    memset(&advParams, 0, sizeof(advParams));
    advParams.conn_mode = BLE_GAP_CONN_MODE_UND;
    advParams.disc_mode = BLE_GAP_DISC_MODE_GEN;
    rc = ble_gap_adv_start(gAddrType, NULL, BLE_HS_FOREVER, &advParams, handleGAPEvent, NULL);
    if (rc != 0) {
        MODLOG_DFLT(ERROR, "error enabling advertisement; rc=%d\n", rc);
        return;
    }
}

void onSync() {
    int rc;

    rc = ble_hs_id_infer_auto(0, &gAddrType);
    assert(rc == 0);

    uint8_t address[6] = {0};
    rc = ble_hs_id_copy_addr(gAddrType, address, NULL);

    MODLOG_DFLT(INFO, "Device Address: ");
    printAddr(address);
    MODLOG_DFLT(INFO, "\n");

    enableAdvertising();
}

void onReset(int reason) {
    MODLOG_DFLT(ERROR, "Resetting state; reason=%d\n", reason);
}

void bleHostTaskMain(void *param) {
    ESP_LOGI(tag, "BLE Host Task Started");
    nimble_port_run(); // This function will return only when nimble_port_stop() is executed
    nimble_port_freertos_deinit();
}

esp_err_t gattCharacterisicAccessHeartRate(uint16_t connHandle, uint16_t attrHandle, struct ble_gatt_access_ctxt* ctxt, void* arg) {
    esp_err_t rc = BLE_ATT_ERR_UNLIKELY;
    if (ble_uuid_u16(ctxt->chr->uuid) == GATT_HRS_BODY_SENSOR_LOC_UUID) {
    	uint8_t loc = GATT_SENSOR_LOCATION_FINGER;
        if ((rc = os_mbuf_append(ctxt->om, &loc, sizeof(loc))) != ESP_OK) {
        	rc = BLE_ATT_ERR_INSUFFICIENT_RES;
        }
    } else {
    	assert(0);
    }
    return rc;
}

esp_err_t gattCharacteristicAccessDeviceInfo(uint16_t connHandle, uint16_t attrHandle, struct ble_gatt_access_ctxt* ctxt, void* arg) {
    esp_err_t rc = BLE_ATT_ERR_UNLIKELY;

    MODLOG_DFLT(INFO, "gattCharacteristicAccessDeviceInfo; connHandle=%d\n attrHandle=%d\n", connHandle, attrHandle);
    uint16_t uuid = ble_uuid_u16(ctxt->chr->uuid);
    if (uuid == GATT_MODEL_NUMBER_UUID) {
        if ((rc = os_mbuf_append(ctxt->om, gModelNum, strlen(gModelNum))) != ESP_OK) {
        	rc = BLE_ATT_ERR_INSUFFICIENT_RES;
        }
    } else if (uuid == GATT_MANUFACTURER_NAME_UUID) {
        if ((rc = os_mbuf_append(ctxt->om, gManufacturerName, strlen(gManufacturerName))) != ESP_OK) {
        	rc = BLE_ATT_ERR_INSUFFICIENT_RES;
        }
    } else {
    	assert(0);
    }
    return rc;
}

esp_err_t initGATTServer() {
    ble_svc_gap_init();
    ble_svc_gatt_init();

    esp_err_t rc = ESP_OK;
    if ((rc = ble_gatts_count_cfg(gGATTServices)) != ESP_OK) {
        return rc;
    }
    if ((rc = ble_gatts_add_svcs(gGATTServices)) != ESP_OK) {
        return rc;
    }
    return ESP_OK;
}

esp_err_t blehrdevice_init() {
	esp_err_t rc = ESP_OK;
    // Initialize NimBLE
    nimble_port_init();
    ble_hs_cfg.sync_cb = onSync;
    ble_hs_cfg.reset_cb = onReset;

    if ((rc = initGATTServer()) != ESP_OK) {
    	return rc;
    }
    assert(rc == 0);

    // Set the default device name; could also be done via sdkconfig
    if ((rc = ble_svc_gap_device_name_set(gDeviceName)) != ESP_OK) {
    	return rc;
    }

    return ESP_OK;
}

void blehrdevice_start() {
	// Start the task
	nimble_port_freertos_init(bleHostTaskMain);
}

void blehrdevice_notifyHeartbeat(int16_t pulse_bpm) {
    static uint8_t hrm[2] = {
		(GATT_SENSOR_HEARTBEAT_FLAG_VALUEFORMAT_UINT8 | GATT_SENSOR_HEARTBEAT_FLAG_SENSORCONTACT_SUPPORTED |
		GATT_SENSOR_HEARTBEAT_FLAG_ENERGYEXPD_NOTPRESENT | GATT_SENSOR_HEARTBEAT_FLAG_RRINTERVAL_NOTPRESENT),
		0
    };

    if (!gNotifyState) { // only if notification is on
        return;
    }

    if (pulse_bpm >= 0) {
    	hrm[0] |= GATT_SENSOR_HEARTBEAT_FLAG_SENSORCONTACT_GOOD;
    	hrm[1] = (pulse_bpm <= 255) ? pulse_bpm : 255; // saturate
    }

    struct os_mbuf* om = ble_hs_mbuf_from_flat(hrm, sizeof(hrm));
    esp_err_t rc = ble_gattc_notify_custom(gConnectionHandle, gHeartRateValueHandle, om);
    assert(rc == 0);
}
