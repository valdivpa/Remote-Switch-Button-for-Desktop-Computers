# Remote Switch Button for Desktop Computers

Turn on a desktop PC remotely from the Blynk app using an Arduino UNO R4 WiFi and a relay wired to the motherboard's front panel power switch pins.

## Wiring

![Fritzing Diagram](Blynk_Connection_Testing/Fritzing%20Diagram.png)

## Sketches

- `Blynk_Arduino_Remote_Switch`: full version with LED matrix animations (WiFi, Blynk and signal strength).
- `Blynk_Arduino_Remote_Switch_No_Bitmaps`: same connection and reconnection logic, without the LED matrix.
- `Blynk_Arduino_Remote_Switch_Simplified`: minimal version, just the Blynk connection and the relay pulse.

All sketches press the power button (500 ms relay pulse on pin 2) when virtual pin `V0` is set to `1`.

## Setup

In each sketch folder, copy `arduino_secrets.h.example` to `arduino_secrets.h` and fill in your Blynk template, auth token and WiFi credentials. `arduino_secrets.h` is ignored by git.
