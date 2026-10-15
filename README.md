# Gym Puck

A small, round remote for music at the gym. It shows what's playing on my iPhone and lets me control it without taking the phone out of my pocket. It also keeps a record of the days I went to the gym.

This is also my project for learning C++ and embedded systems.

## How it works

The puck connects to the iPhone over Bluetooth Low Energy using Apple Media Service (AMS), which is built into iOS. It receives the current track's details and sends playback commands back. No iPhone app is needed.

## Hardware

- Waveshare ESP32-S3-Touch-AMOLED-1.32: ESP32-S3 with a 1.32" round AMOLED touch display (466 × 466)
- LiPo battery (to add)
- 3D-printed case (to design)

## Software

C++ with the Arduino framework, built with PlatformIO in VS Code. The interface uses LVGL.

## Roadmap

- [x] Run Waveshare's demo to confirm the display and touch work
- [x] Show text on the screen
- [x] Read touch and the onboard buttons
- [ ] Build a round UI with LVGL
- [ ] Pair with the iPhone and show the current track
- [ ] Add playback controls
- [ ] Add the gym tracker
- [ ] Add a battery and deep sleep
- [ ] Design and print the case
