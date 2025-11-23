#ifndef DISPLAY_ILI9341_H
#define DISPLAY_ILI9341_H

#include <Adafruit_ILI9341.h>
#include <Adafruit_GFX.h>

class DisplayILI9341 {
private:
    Adafruit_ILI9341 tft;
    int8_t backlightPin;
    uint8_t brightness;

public:
    // DisplayILI9341(int8_t cs, int8_t dc, int8_t rst, int8_t bl);
    DisplayILI9341(int8_t cs, int8_t dc, int8_t rst, int8_t mosi, int8_t miso, int8_t sclk, int8_t bl);
    
    void begin();
    void clear();
    void setBrightness(uint8_t level);
    
    void drawText(int16_t x, int16_t y, const char* text, uint16_t color, uint8_t size = 1);
    void drawTime(int16_t x, int16_t y, uint8_t hour, uint8_t minute, uint8_t second);
    void drawDate(int16_t x, int16_t y, uint16_t year, uint8_t month, uint8_t day);
    void drawAlarmStatus(int16_t x, int16_t y, bool enabled, uint8_t hour, uint8_t minute);
    void drawFMFrequency(int16_t x, int16_t y, float frequency);
    
    void drawBitmap(int16_t x, int16_t y, const uint16_t* bitmap, int16_t w, int16_t h);
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    
    int16_t getWidth();
    int16_t getHeight();
};

#endif