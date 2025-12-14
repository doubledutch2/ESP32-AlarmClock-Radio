#ifndef DISPLAY_OLED_H
#define DISPLAY_OLED_H

#include "DisplayInterface.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1

class DisplayOLED : public DisplayInterface {
private:
    Adafruit_SSD1306 display;
    int sdaPin;
    int sclPin;
    DisplayMode currentMode;
    uint8_t brightness;
    bool initialized;
    
    void drawClockAnalog(uint8_t hour, uint8_t minute, uint8_t second);
    void drawClockDigital(uint8_t hour, uint8_t minute, uint8_t second);

public:
    DisplayOLED(int sda, int scl);
    
    bool begin() override;
    void clear() override;
    void update() override;
    
    void setMode(DisplayMode mode) override;
    DisplayMode getMode() override;
    
    void setBrightness(uint8_t level) override;
    uint8_t getBrightness() override;
    
    void drawClock(uint8_t hour, uint8_t minute, uint8_t second) override;
    void drawDate(uint16_t year, uint8_t month, uint8_t day) override;
    
    void showInternetRadio(const char* stationName, const char* artist, const char* title) override;
    void showFMRadio(const char* frequency, const char* rdsStation, const char* rdsText) override;
    
    void drawInfo(int volume, int brightness) override;
    void showStartupMessage(const char* message) override;
    void showError(const char* message) override;
    
    void drawText(int16_t x, int16_t y, const char* text, uint16_t color, uint8_t size = 1) override;
};

#endif