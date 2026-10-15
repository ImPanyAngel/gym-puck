#include <Arduino.h>
#include <NimBLEDevice.h>
#include "ams.h"
#include "ui.h"

// Apple Media Service and its characteristics, from Apple's AMS specification
static const char * AMS_SERVICE_UUID       = "89D3502B-0F36-433A-8EF4-C502AD55F8DC";
static const char * AMS_ENTITY_UPDATE_UUID = "2F7CABCE-808D-411F-9A0C-BB92BA96C102";
static const char * AMS_REMOTE_COMMAND_UUID = "9B3C81D8-57B1-4A8A-B8DF-0E56F7CA51C2";

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

// Everything that talks to the phone happens on this one task, fed by a queue. Asking the
// phone something means waiting for its reply, which can't be done inside the Bluetooth
// stack's callbacks (it would be waiting for itself) or LVGL's (the screen would freeze).
// Using a single task also means subscribing and sending commands can never overlap.
enum AmsMessageType : uint8_t {
    MSG_CONNECTED,      // value: connection handle
    MSG_DISCONNECTED,
    MSG_COMMAND,        // value: an AmsCommand
};

struct AmsMessage {
    AmsMessageType type;
    uint16_t value;
};

static QueueHandle_t message_queue;
static NimBLERemoteCharacteristic * remote_command = nullptr;   // only used on the AMS task

static void subscribe(uint16_t conn_handle) {
    // Getting the client clears whatever was found on a previous connection
    NimBLEClient * client = NimBLEDevice::getServer()->getClient(conn_handle);

    NimBLERemoteService * ams = client->getService(AMS_SERVICE_UUID);
    if (ams == nullptr) {
        Serial.println("AMS: service not found on the phone");
        return;
    }

    NimBLERemoteCharacteristic * entity_update = ams->getCharacteristic(AMS_ENTITY_UPDATE_UUID);
    if (entity_update == nullptr || !entity_update->subscribe(true, on_entity_update)) {
        Serial.println("AMS: could not subscribe to Entity Update");
        return;
    }

    // Tell the phone what to send. It replies with the current values straight away,
    // then sends each one again whenever it changes.
    const uint8_t track_request[] = {ENTITY_TRACK, TRACK_ARTIST, TRACK_TITLE, TRACK_DURATION};
    const uint8_t player_request[] = {ENTITY_PLAYER, PLAYER_PLAYBACK_INFO};
    bool ok = entity_update->writeValue(track_request, sizeof(track_request), true) &&
              entity_update->writeValue(player_request, sizeof(player_request), true);
    Serial.println(ok ? "AMS: subscribed" : "AMS: subscription request failed");
    if (!ok) {
        return;
    }

    remote_command = ams->getCharacteristic(AMS_REMOTE_COMMAND_UUID);
    if (remote_command == nullptr) {
        Serial.println("AMS: Remote Command not found, controls won't work");
    }
    ui_set_connected(true);
}

static void send_command(uint8_t command) {
    if (remote_command == nullptr) {
        Serial.printf("AMS: not connected, command %d ignored\n", command);
        return;
    }
    // One byte: the command number. The phone acts on it and sends any resulting changes
    // (new track, paused...) through Entity Update, which updates the screen.
    bool ok = remote_command->writeValue(&command, 1, true);
    Serial.printf("AMS: command %d %s\n", command, ok ? "sent" : "failed");
}

static void ams_task(void * param) {
    AmsMessage message;
    while (true) {
        xQueueReceive(message_queue, &message, portMAX_DELAY);   // sleeps until a message
        switch (message.type) {
            case MSG_CONNECTED:
                subscribe(message.value);
                break;
            case MSG_DISCONNECTED:
                remote_command = nullptr;   // it belonged to the old connection
                break;
            case MSG_COMMAND:
                send_command(message.value);
                break;
        }
    }
}

// Queues a message without waiting. If the queue is ever full, the message is dropped,
// which for a button tap is better than freezing whoever called this.
static void post(AmsMessageType type, uint16_t value) {
    AmsMessage message = {type, value};
    xQueueSend(message_queue, &message, 0);
}

void ams_init(void) {
    message_queue = xQueueCreate(8, sizeof(AmsMessage));
    xTaskCreate(ams_task, "ams", 8 * 1024, NULL, 2, NULL);
}

void ams_start(uint16_t conn_handle) {
    post(MSG_CONNECTED, conn_handle);
}

void ams_stop(void) {
    post(MSG_DISCONNECTED, 0);
}

void ams_send_command(AmsCommand command) {
    post(MSG_COMMAND, command);
}
