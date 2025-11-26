#ifndef CONFIG_H
#define CONFIG_H

// ===== Feature Flags =====
// Choose which display to use (comment/uncomment)
// #define USE_OLED_DISPLAY    // Use I2C OLED display
#define USE_TFT_DISPLAY  // Use SPI TFT display (ILI9341)

// Enable/disable modules
#define ENABLE_AUDIO     true
#define ENABLE_LED       true
#define ENABLE_WEB       true
#define ENABLE_FM_RADIO  true  // Set to true now that we're using FM

// Audio Configuration
#define ENABLE_STEREO    true   // true = 2 speakers, false = 1 speaker

// ===== Pin Definitions for ESP32-S3-DevKitC-1 =====
// *** LOCKED - DO NOT CHANGE THESE PINS ***

// I2C pins (Shared by OLED and RDA5807)
#define I2C_SDA          8
#define I2C_SCL          9

// uint8_t rdaAddresses[] = {0x10, 0x11, 0};

// I2S Audio pins (for Internet Radio via MAX98357A)
// STEREO: Use two MAX98357A amplifiers
#define I2S_LRC          7   // Word Select (shared for both amps)
#define I2S_BCLK         15  // Bit Clock (shared for both amps)
#define I2S_DOUT_L       16  // Data Out LEFT channel (to first MAX98357A)
#define I2S_DOUT_R       17  // Data Out RIGHT channel (to second MAX98357A)

// SPI pins for TFT Display (ILI9341)
#define TFT_MISO         12  
#define TFT_MOSI         11  
#define TFT_SCLK         13  
#define TFT_CS           10  
#define TFT_DC           14  
#define TFT_RST          21  
#define TFT_BL           47  // Backlight PWM

// Control pins
#define VOL_PIN          1   // ADC for volume potentiometer
#define LED_PIN          48  // RGB LED (built-in on DevKitC)
#define BRIGHTNESS_PIN   38  // Brightness button
#define NEXT_STATION_PIN 39  // Next station button
#define MODE_SWITCH_PIN  40  // Switch between Internet/FM radio

// FM Radio pins (only used if ENABLE_FM_RADIO is true)
#if ENABLE_FM_RADIO
  #define FM_RESET_PIN     42  // Si4735 reset pin
  #define AUDIO_SWITCH_PIN 41  // Audio source switching (HIGH=FM, LOW=Internet)
#endif

// Buzzer and Button Pins
#define BUZZER_PIN  25

// Button Pins for alarm clock interface
#define BTN_UP      3
#define BTN_DOWN    4
#define BTN_SELECT  5
#define BTN_SNOOZE  6

// *** END OF LOCKED PINS ***

// ===== WiFi Settings =====
#define WIFI_SSID        "DEBEER"
#define WIFI_PASSWORD    "B@C&86j@gqW73g"
#define MDNS_NAME        "alarmclock"

// ===== Audio Settings =====
#define MAX_VOLUME       25

// ===== Time Settings =====
#define GMT_OFFSET       0   // GMT offset in hours
#define DST_OFFSET       0   // DST offset in hours

// NEW: Time settings in seconds (required by TimeModule)
#define GMT_OFFSET_SEC       0        // GMT offset in seconds (0 = GMT)
#define DAYLIGHT_OFFSET_SEC  3600     // DST offset in seconds (3600 = +1 hour for BST)

// ===== LED Settings =====
#define BRIGHT_FULL      250
#define BRIGHT_DIM       5

// ===== Storage Settings =====
#define MAX_STATIONS     50
#define MAX_ALARMS       10

#endif