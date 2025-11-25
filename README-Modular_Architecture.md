# ESP32 Alarm Clock Radio - Modular Architecture

## File Structure

```
AlarmClock/
├── AlarmClock.ino              # Main file (~100 lines)
├── Config.h                    # Pin definitions and settings
├── CommonTypes.h               # Shared data structures
│
├── Hardware Management
│   ├── HardwareSetup.h
│   └── HardwareSetup.cpp       # Initializes all hardware modules
│
├── UI/Menu System
│   ├── MenuSystem.h
│   ├── MenuSystem.cpp          # Menu logic and button handling
│   └── MenuSystem_Screens.cpp  # Screen drawing functions
│
├── Alarm Logic
│   ├── AlarmController.h
│   └── AlarmController.cpp     # Alarm trigger/snooze logic
│
├── Display Modules
│   ├── DisplayInterface.h      # Abstract base class
│   ├── DisplayILI9341.h/.cpp   # TFT implementation
│   └── DisplayOLED.h/.cpp      # OLED implementation
│
├── Hardware Modules
│   ├── TimeModule.h/.cpp       # WiFi + NTP time
│   ├── FMRadioModule.h/.cpp    # RDA5807 FM radio
│   ├── BuzzerModule.h/.cpp     # Alarm buzzer
│   ├── StorageModule.h/.cpp    # NVS + LittleFS storage
│   ├── WiFiModule.h/.cpp       # WiFi management
│   ├── AudioModule.h/.cpp      # Internet radio streaming
│   ├── WebServerModule.h/.cpp  # Web configuration interface
│   └── LEDModule.h/.cpp        # Status LED
```

## Key Features

### 1. Separation of Concerns
- **AlarmClock.ino**: Just initialization and main loop
- **HardwareSetup**: All hardware initialization
- **MenuSystem**: All UI/menu logic
- **AlarmController**: Alarm-specific logic

### 2. No More Conflicts
- **InternetRadioStation**: For streaming URLs
- **FMRadioPreset**: For FM radio presets
- Defined once in CommonTypes.h

### 3. Display Abstraction
- **DisplayInterface**: Abstract base class
- **DisplayILI9341**: TFT implementation (with analog clock)
- **DisplayOLED**: OLED implementation
- Easy to switch between displays

### 4. Easy to Maintain
- Update menu screens? Edit MenuSystem_Screens.cpp
- Change hardware? Edit HardwareSetup.cpp
- Modify alarm logic? Edit AlarmController.cpp
- Main file stays clean and simple

## Compiling

### Required Libraries
Install via Arduino Library Manager:
- Adafruit_ILI9341
- Adafruit_GFX
- Adafruit_SSD1306 (for OLED)
- Adafruit_NeoPixel
- ESP32-Audio-I2S (by schreibfaul1)
- PU2CLR RDA5807

### MenuSystem Files
**Important**: You need to compile BOTH MenuSystem.cpp files together:

**Option 1**: Combine them into one file
```cpp
// MenuSystem.cpp
#include "MenuSystem.h"
// ... core functions ...
// ... paste MenuSystem_Screens.cpp content here ...
```

**Option 2**: Keep separate and add to project
Just make sure both .cpp files are in your sketch folder

## Configuration

### WiFi Settings (Config.h)
```cpp
#define WIFI_SSID "DEBEER"
#define WIFI_PASSWORD "B@C&86j@gqW73g"
```

### Time Zone (Config.h)
```cpp
#define GMT_OFFSET_SEC 0        // GMT
#define DAYLIGHT_OFFSET_SEC 3600 // +1 hour BST
```

### Pin Assignments
All pins are defined in Config.h - review and adjust for your hardware

## Web Interface

Once running, access the web interface at:
- `http://alarmclock.local`
- Or use the IP address shown on the display

Features:
- Play custom internet radio stations
- Add stations to presets
- Control playback

## Usage

### Button Controls
- **UP/DOWN**: Navigate menus
- **SELECT**: Choose/confirm
- **SNOOZE**: Snooze alarm (5 minutes)
- **BRIGHTNESS**: Cycle display brightness
- **NEXT STATION**: Skip to next station
- **VOLUME POT**: Adjust audio volume

### Menus
1. **Main**: Shows clock with analog + digital display
2. **Set Time**: Adjust time manually (syncs with NTP)
3. **Set Alarm**: Configure alarm time
4. **FM Radio**: Tune FM radio
5. **Stations**: Browse and play saved stations
6. **Settings**: View system status

## Troubleshooting

### Display shows "RadioStation redefinition"
Make sure you're using the new CommonTypes.h with separate types

### MenuSystem won't compile
Combine MenuSystem.cpp and MenuSystem_Screens.cpp into one file

### Audio not working
Check ESP32-Audio-I2S library is installed and I2S pins are correct

### FM Radio not working
Verify RDA5807 module is connected to correct I2C pins (SDA=8, SCL=9)

## Future Enhancements

Easy to add:
- More display types (just implement DisplayInterface)
- Additional menu screens (add to MenuSystem)
- New alarm sounds (modify AlarmController)
- More radio presets (edit defaultStations array)
- Custom web UI (modify WebServerModule)
