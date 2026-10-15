#include <Arduino.h>
#include <NimBLEDevice.h>
#include "ble.h"
#include "ams.h"
#include "ui.h"

static const char * DEVICE_NAME = "Gym Puck";

// Apple Media Service, which runs on the iPhone. Asking for it in the advertisement
// ("soliciting" it) is what makes iOS list the puck in Settings > Bluetooth.
static const char * AMS_SERVICE_UUID = "89D3502B-0F36-433A-8EF4-C502AD55F8DC";

// Advertising data type for "List of 128-bit Service Solicitation UUIDs"
static const uint8_t AD_TYPE_SOLICIT_128 = 0x15;

// These run on the Bluetooth stack's own task, not loop() or LVGL's task
class ServerCallbacks : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer * server, NimBLEConnInfo & info) override {
        Serial.printf("BLE connected: %s\n", info.getAddress().toString().c_str());
        // Ask for an encrypted link. The first time, this makes iOS show the pairing prompt;
        // after that, the stored keys are reused silently.
        NimBLEDevice::startSecurity(info.getConnHandle());
    }

    void onDisconnect(NimBLEServer * server, NimBLEConnInfo & info, int reason) override {
        Serial.printf("BLE disconnected, reason %d\n", reason);
        ui_set_connected(false);
    }

    void onAuthenticationComplete(NimBLEConnInfo & info) override {
        Serial.printf("BLE security: encrypted %s, bonded %s\n",
                      info.isEncrypted() ? "yes" : "no",
                      info.isBonded() ? "yes" : "no");
        // AMS only answers over an encrypted link
        if (info.isEncrypted()) {
            ams_start(info.getConnHandle());
        }
    }
};

static ServerCallbacks server_callbacks;

void ble_init(void) {
    NimBLEDevice::init(DEVICE_NAME);

    // Allow packets up to 185 bytes (the default is 23), so most titles fit in one
    // notification instead of arriving truncated. The phone agrees to this when it connects.
    NimBLEDevice::setMTU(185);

    // Bonding: remember the pairing, so the iPhone reconnects without asking again.
    // No passkey (the puck has no keyboard), secure connections on.
    NimBLEDevice::setSecurityAuth(true, false, true);
    NimBLEDevice::setSecurityIOCap(BLE_HS_IO_NO_INPUT_OUTPUT);

    NimBLEServer * server = NimBLEDevice::createServer();
    server->setCallbacks(&server_callbacks, false);
    server->advertiseOnDisconnect(true);   // be findable again after the phone disconnects

    // Advertisement: discoverable, Bluetooth LE only, soliciting AMS.
    // A UUID goes over the air as its 16 bytes in reverse order, which is how NimBLEUUID stores it.
    NimBLEUUID ams(AMS_SERVICE_UUID);
    NimBLEAdvertisementData adv_data;
    adv_data.setFlags(BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP);
    uint8_t solicit[18] = {17, AD_TYPE_SOLICIT_128};   // length (type + 16 bytes), then type
    memcpy(&solicit[2], ams.getValue(), 16);
    adv_data.addData(solicit, sizeof(solicit));

    // The name doesn't fit in the advertisement as well, so it goes in the scan response
    NimBLEAdvertisementData scan_data;
    scan_data.setName(DEVICE_NAME);

    NimBLEAdvertising * advertising = NimBLEDevice::getAdvertising();
    advertising->setAdvertisementData(adv_data);
    advertising->setScanResponseData(scan_data);
    advertising->start();

    Serial.println("BLE advertising as \"Gym Puck\"");
}
