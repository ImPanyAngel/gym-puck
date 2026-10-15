#pragma once

// Starts Bluetooth and advertises the puck so the iPhone can pair with it.
// After the first pairing, the iPhone reconnects by itself.
void ble_init(void);
