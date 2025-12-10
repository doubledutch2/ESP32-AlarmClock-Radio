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

## Partitioning

You need a partition scheme for the ESP32-S3-N16R8 which allows for 3Mb App, LittleFS. Creata file called, which is stored in partitions.csv which is in the .ino directory

- The IDE setup is critical (Arduino -> Tools)
- USB CDC	On Boot: Enabled
- CPU Frequency: 240MHz
- Core Debug Level: None
- USB DFU On Boot: Disabled
- Erase All Flash Before Sketch Upload: Enabled
- Events Run On: Core 1
- Flash Mode: QIO 80MHz
- Flash Size: 16MB (128Mb)
- JTAG Adapter: Disabled
- Arduino Runs on: Core 1
- USB Firmware MSC On Boot: Disabled
- Partition Scheme: Custom (see above partition.csv file)
- PSRAM: OPI PSRAM
- Upload Mode: UART0/Hardware CDC
- Upload Speed: 921600
- USB Mode: Hardware CDC and JTAG
- Zigbee Mode: Disabled

## PINOUT

Pin Assignments ESP32

 0 - <br/>
 1 - VOL_POT

 2 -

 3 - BTN_UP

 4 - BTN_DOWN

 5 - BTN_SELECT 

 6 - BTN_SNOOZE

 7 - I2S_LCR

 8 - I2C_SCA

 9 - I2C_SCL

10 - TFT_CS

11 - TFT_MOSI

12 - TFT_MISO

13 - 

14 - 

15 - I2S_BCLK

16 - I2S_DOUT

17 - MODE_SWITCH_PIN

18 - [Reserved] ?????

19 - [Reserved] USB D- (Do not Use)

20 - [Reserved] USB D+ (Do not Use)

21 - TFT_RST

35 - [Reserved PSRAM]

36 - [Reserved PSRAM]

37 - [Reserved PSRAM]

38 - BTN_BRIGHT

39 - BTN_NEXT_STATION

40 - TFT_SCLK [MODE_SWITCH

41 - AUDIO_SWITCH

42 - FM_RESET

45 - 

47 - TFT_BL

48 - LED Build In


## Software Requirements

- Arduino IDE /2.x
- ESP32 Board Package
- Required Libraries:
  - TFT_eSPI
  - ESP32-audioI2S
  - Adafruit NeoPixel
  - SI4735 Arduino Library (optional)

## Compile with new ESP32 Board
- Connect USB cable to COM Port
- Upload simple Blink Sketch
- Change USB Cable to USB Port
- Enable "USB CDC On Boot"
- Use ESP32S3 Dev Module"
- No button press required

## Installation

1. Clone this repository
2. Open `firmware/AlarmClock/AlarmClock.ino` in Arduino IDE
3. Configure `config.h` with your WiFi credentials
4. Configure TFT_eSPI User_Setup.h (see docs/SETUP.md)
5. Upload to ESP32-S3


## Wave Files

They are stored in the SketchDirectory/data/Alarmsounds folder. Use  this tool to upload them:

https://randomnerdtutorials.com/arduino-ide-2-install-esp32-littlefs/

When installed use <command><shift><p> and function Upload LittleFS

## Documentation

- [Wiring Guide](docs/WIRING.md)
- [Bill of Materials](docs/BOM.md)
- [Setup Instructions](docs/SETUP.md)

## License

MIT License (or your choice)

## Version

1.0 - Initial Release
