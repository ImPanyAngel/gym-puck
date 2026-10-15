#include <Arduino.h>
#include <NimBLEDevice.h>
#include "ams.h"
#include "ui.h"

// Apple Media Service and its characteristics, from Apple's AMS specification
static const char * AMS_SERVICE_UUID       = "89D3502B-0F36-433A-8EF4-C502AD55F8DC";
static const char * AMS_ENTITY_UPDATE_UUID = "2F7CABCE-808D-411F-9A0C-BB92BA96C102";

// Entities: the "things" AMS reports on
static const uint8_t ENTITY_PLAYER = 0;
static const uint8_t ENTITY_TRACK  = 2;

// Attributes of the player
static const uint8_t PLAYER_PLAYBACK_INFO = 1;   // "state,rate,elapsed", e.g. "1,1.0,42.5"

// Playback states in that first field (2 and 3 are rewinding and fast-forwarding)
static const int PLAYBACK_STATE_PLAYING = 1;

// Attributes of the track
static const uint8_t TRACK_ARTIST   = 0;
static const uint8_t TRACK_TITLE    = 2;
static const uint8_t TRACK_DURATION = 3;         // seconds, as text, e.g. "225.4"

// Entity Update flag: the value was cut short to fit in one notification
static const uint8_t FLAG_TRUNCATED = 0x01;

static const char * attribute_name(uint8_t entity, uint8_t attribute) {
    if (entity == ENTITY_PLAYER && attribute == PLAYER_PLAYBACK_INFO) return "Playback";
    if (entity == ENTITY_TRACK && attribute == TRACK_ARTIST)          return "Artist";
    if (entity == ENTITY_TRACK && attribute == TRACK_TITLE)           return "Title";
    if (entity == ENTITY_TRACK && attribute == TRACK_DURATION)        return "Duration";
    return "Other";
}

// A notification is: entity (1 byte), attribute (1 byte), flags (1 byte), then the
// value as text, not zero-terminated. Runs on the Bluetooth stack's task.
static void on_entity_update(NimBLERemoteCharacteristic * characteristic, uint8_t * data, size_t length, bool is_notify) {
    if (length < 3) {
        return;
    }
    uint8_t entity = data[0];
    uint8_t attribute = data[1];
    uint8_t flags = data[2];
    const char * value = (const char *) &data[3];
    int value_length = length - 3;

    // %.*s prints exactly value_length characters, as the text has no terminating zero
    Serial.printf("AMS %-8s: %.*s%s\n", attribute_name(entity, attribute),
                  value_length, value, (flags & FLAG_TRUNCATED) ? " (truncated)" : "");

    // Copy into a zero-terminated string, cutting it short if it doesn't fit
    char text[192];
    if (value_length > (int) sizeof(text) - 1) {
        value_length = sizeof(text) - 1;
    }
    memcpy(text, value, value_length);
    text[value_length] = '\0';

    if (entity == ENTITY_TRACK && attribute == TRACK_TITLE) {
        ui_set_title(text);
    } else if (entity == ENTITY_TRACK && attribute == TRACK_ARTIST) {
        ui_set_artist(text);
    } else if (entity == ENTITY_TRACK && attribute == TRACK_DURATION) {
        ui_set_duration(atof(text));   // atof gives 0 for empty text
    } else if (entity == ENTITY_PLAYER && attribute == PLAYER_PLAYBACK_INFO) {
        // "state,rate,elapsed". Fields can be empty (e.g. "0,,"), in which case sscanf
        // stops early and the remaining values keep their defaults of 0.
        int state = 0;
        float rate = 0;
        float elapsed = 0;
        sscanf(text, "%d,%f,%f", &state, &rate, &elapsed);
        ui_set_playback(state == PLAYBACK_STATE_PLAYING, rate, elapsed);
    }
}

// Asking the phone things means waiting for its replies, which can't be done inside the
// Bluetooth stack's own callbacks (it would be waiting for itself), so this runs as a task.
static void ams_subscribe_task(void * param) {
    uint16_t conn_handle = (uint16_t)(uintptr_t) param;
    NimBLEClient * client = NimBLEDevice::getServer()->getClient(conn_handle);

    NimBLERemoteService * ams = client->getService(AMS_SERVICE_UUID);
    if (ams == nullptr) {
        Serial.println("AMS: service not found on the phone");
        vTaskDelete(NULL);
        return;
    }

    NimBLERemoteCharacteristic * entity_update = ams->getCharacteristic(AMS_ENTITY_UPDATE_UUID);
    if (entity_update == nullptr || !entity_update->subscribe(true, on_entity_update)) {
        Serial.println("AMS: could not subscribe to Entity Update");
        vTaskDelete(NULL);
        return;
    }

    // Tell the phone what to send. It replies with the current values straight away,
    // then sends each one again whenever it changes.
    const uint8_t track_request[] = {ENTITY_TRACK, TRACK_ARTIST, TRACK_TITLE, TRACK_DURATION};
    const uint8_t player_request[] = {ENTITY_PLAYER, PLAYER_PLAYBACK_INFO};
    bool ok = entity_update->writeValue(track_request, sizeof(track_request), true) &&
              entity_update->writeValue(player_request, sizeof(player_request), true);
    Serial.println(ok ? "AMS: subscribed" : "AMS: subscription request failed");
    if (ok) {
        ui_set_connected(true);
    }

    vTaskDelete(NULL);
}

void ams_start(uint16_t conn_handle) {
    xTaskCreate(ams_subscribe_task, "ams_subscribe", 8 * 1024, (void *)(uintptr_t) conn_handle, 2, NULL);
}
