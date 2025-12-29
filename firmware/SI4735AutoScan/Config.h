#ifndef CONFIG_H
#define CONFIG_H


// Add this to the top of DisplayILI9341.h or Config.h to maintain compatibility

// TFT_eSPI uses TFT_ prefix for colors instead of ILI9341_
// Define aliases for backward compatibility

#ifndef TFT_COLOR_COMPAT_H
#define TFT_COLOR_COMPAT_H

#define ILI9341_BLACK       TFT_BLACK
#define ILI9341_NAVY        TFT_NAVY
#define ILI9341_DARKGREEN   TFT_DARKGREEN
#define ILI9341_DARKCYAN    TFT_DARKCYAN
#define ILI9341_MAROON      TFT_MAROON
#define ILI9341_PURPLE      TFT_PURPLE
#define ILI9341_OLIVE       TFT_OLIVE
#define ILI9341_LIGHTGREY   TFT_LIGHTGREY
#define ILI9341_DARKGREY    TFT_DARKGREY
#define ILI9341_BLUE        TFT_BLUE
#define ILI9341_GREEN       TFT_GREEN
#define ILI9341_CYAN        TFT_CYAN
#define ILI9341_RED         TFT_RED
#define ILI9341_MAGENTA     TFT_MAGENTA
#define ILI9341_YELLOW      TFT_YELLOW
#define ILI9341_WHITE       TFT_WHITE
#define ILI9341_ORANGE      TFT_ORANGE
#define ILI9341_GREENYELLOW TFT_GREENYELLOW
#define ILI9341_PINK        TFT_PINK

#endif

// ===== Feature Flags =====
// Choose which display to use (comment/uncomment)
// #define USE_OLED_DISPLAY    // Use I2C OLED display
#define USE_TFT_DISPLAY  // Use SPI TFT display (ILI9341)

#define ENABLE_TOUCHSCREEN  true
#define ENABLE_BUTTONS      true
#define ENABLE_DRAW         true
#define ENABLE_AUDIO        true
#define ENABLE_STEREO       true
#define ENABLE_LED          true
#define ENABLE_ALARMS       true
#define ENABLE_WEB          true
#define ENABLE_FM_RADIO     true  // Set to true now that we're using FM
#define ENABLE_PRAM         true
#define ENABLE_I2C_SCAN     true

// ===== Pin Definitions for ESP32-S3-DevKitC-1 =====
// *** LOCKED - DO NOT CHANGE THESE PINS ***

// I2C pins (Shared by OLED and RDA5807)
#define I2C_SDA          21 // Green
#define I2C_SCL          18 // Yellow

// uint8_t rdaAddresses[] = {0x10, 0x11, 0};7



// I2S Audio pins (for Internet Radio via MAX98357A)
#define I2S_LRC          7    // (Green)  To pin 5 ---Word Select (shared for both amps)
#define I2S_BCLK         8    // (Yellow) To Pin 2 --- Bit Clock (shared for both amps)
#define I2S_DOUT         16   // (Blue)   To Pin 12 --- Data Out LEFT channel (to first MAX98357A)
/*
Pin 40 Currently: 

Yellow Black: Screen SCK -> Move to 13
White: T_CLK

Pin 13 Currently:

Green: T_CS -> move to

*/
/* NEW  - https://forum.arduino.cc/t/esp32-touchscreen-tft_espi-ili9341/607951/3 */
// Watch this Video which talks exactly about our setup: https://www.youtube.com/watch?v=_0tgx8tezXU

// #define TOUCH_IRQ 	n/a
#define TOUCH_DO 	13
#define TOUCH_DIN 11
#define TOUCH_CS 	46 // 33 
#define TOUCH_CLK 12

#define TFT_MISO	13
#define TFT_LED		47
#define TFT_SCLK	12
#define TFT_MOSI	11
#define TFT_DC		15	
#define TFT_RST	  9
#define TFT_CS		4
// #define TFT_GND		GND
// #define TFT_VCC		3.3V

// Control pins
#define VOL_PIN          1   // ADC for volume potentiometer
#define LED_PIN          48  // RGB LED (built-in on DevKitC)
#define BRIGHTNESS_PIN   38  // 42 Brightness button
#define NEXT_STATION_PIN 39 // 42  // Next station button

// Switch pin for TS5A3159
// The Mode_Switch_Pin is connected to the IN Pin. 
// If it's HIGH NC = connected to COM // I2C - MP3/Internet Radio
// If it's LOW  NO = connected to COM

#define ENABLE_TOUCHSCREEN     true
#define INIT_TOUCHSCREEN_FIRST false

#define MODE_SWITCH_PIN  17  // 42 Switch between Internet/FM radio

// FM Radio pins (only used if ENABLE_FM_RADIO is true)
#define FM_RESET_PIN     42  // White Si4735 reset pin
#define AUDIO_SWITCH_PIN 41  // Audio source switching (HIGH=FM, LOW=Internet)
#define FM_RCLK_PIN      45  // Blue - 32,768 Hz cloced use by the Si4735 



// Button Pins for alarm clock interface
#define BTN_UP      3
#define BTN_DOWN    6
#define BTN_SELECT  5
#define BTN_SNOOZE  6
#define BTN_SETUP   14  // Setup/Settings button

// *** END OF LOCKED PINS ***

// ===== WiFi Settings =====
#define WIFI_SSID        "DEBEER"
#define WIFI_PASSWORD    "B@C&86j@gqW73g"
#define MDNS_NAME        "alarmclock"

// ===== Audio Settings =====
#define MAX_VOLUME       25

// ===== Time Settings =====
// NOTE: These are DEFAULT values only
// Actual values are loaded from NVS storage and can be changed via web interface
// If no values are saved in NVS, these defaults will be used
#define DEFAULT_GMT_OFFSET_SEC       0        // GMT offset in seconds (0 = GMT/UTC)
#define DEFAULT_DAYLIGHT_OFFSET_SEC  0        // DST offset in seconds (3600 = +1 hour)

// For compatibility with existing code
#define GMT_OFFSET_SEC       DEFAULT_GMT_OFFSET_SEC
#define DAYLIGHT_OFFSET_SEC  DEFAULT_DAYLIGHT_OFFSET_SEC

// ===== LED Settings =====
#define BRIGHT_FULL      250
#define BRIGHT_DIM       5

// ===== Storage Settings =====
#define MAX_STATIONS     50
#define MAX_ALARMS       10

#endif
