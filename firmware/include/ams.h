#pragma once

#include <stdint.h>

// Apple Media Service client: the puck asks the iPhone for now-playing details and
// sends it playback commands. All the talking to the phone happens on a task of its own,
// so every function here returns straight away and is safe to call from any task.

// Command numbers from Apple's AMS specification
enum AmsCommand : uint8_t {
    AMS_COMMAND_TOGGLE_PLAY_PAUSE = 2,
    AMS_COMMAND_NEXT_TRACK        = 3,
    AMS_COMMAND_PREVIOUS_TRACK    = 4,
    AMS_COMMAND_VOLUME_UP         = 5,
    AMS_COMMAND_VOLUME_DOWN       = 6,
};

void ams_init(void);                      // once, at start-up
void ams_start(uint16_t conn_handle);     // the connection is now encrypted
void ams_stop(void);                      // the phone disconnected
void ams_send_command(AmsCommand command);
