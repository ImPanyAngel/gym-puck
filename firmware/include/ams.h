#pragma once

#include <stdint.h>

// Apple Media Service client: the puck asks the iPhone for now-playing details.
// Call once the connection is encrypted; it does its work on a task of its own.
void ams_start(uint16_t conn_handle);
