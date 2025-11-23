# ESP32-S3 Alarm Clock Radio

Internet radio alarm clock with FM/AM radio, TFT display, and web interface.

## Features

- 🌐 Internet radio streaming (thousands of stations)
- 📻 FM/AM radio (optional Si4735 module)
- 🖥️ 2.8" TFT color display (240x320)
- 🔊 Stereo audio output (2x 3W speakers)
- 📱 Web interface for control
- 💾 Non-volatile storage for settings
- 🌙 Day/night display modes
- ⏰ NTP time synchronization

## Hardware

- ESP32-S3-DevKitC-1 (16MB Flash, 8MB PSRAM)
- ILI9341 2.8" TFT Display (SPI)
- 2x MAX98357A I2S Amplifiers
- Si4735 FM/AM Radio Module (optional)
- 2x 4Ω 3W Speakers

## Software Requirements

- Arduino IDE 2.x
- ESP32 Board Package
- Required Libraries:
  - TFT_eSPI
  - ESP32-audioI2S
  - Adafruit NeoPixel
  - SI4735 Arduino Library (optional)

## Installation

1. Clone this repository
2. Open `firmware/AlarmClock/AlarmClock.ino` in Arduino IDE
3. Configure `config.h` with your WiFi credentials
4. Configure TFT_eSPI User_Setup.h (see docs/SETUP.md)
5. Upload to ESP32-S3

## Arduino IDE Settings
Board: ESP32S3 Dev Module
USB CDC On Boot: Enabled
USB Mode: Hardware CDC and JTAG
Flash Size: 16MB
Partition Scheme: Default 4MB with spiffs (1.2MB APP/1.5MB SPIFFS)
PSRAM: QSPI PSRAM

## Documentation

- [Wiring Guide](docs/WIRING.md)
- [Bill of Materials](docs/BOM.md)
- [Setup Instructions](docs/SETUP.md)

## License

MIT License (or your choice)

## Version

1.0 - Initial Release
