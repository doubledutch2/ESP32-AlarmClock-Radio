#ifndef DISPLAY_ILI9341_H
#define DISPLAY_ILI9341_H

#include <SPI.h> // LdB
#include <Adafruit_ILI9341.h>
#include <Adafruit_GFX.h>

class DisplayILI9341 {
private:
    Adafruit_ILI9341 tft;
    int8_t backlightPin;
    uint8_t brightness;
    
    // Cache for preventing unnecessary redraws
    uint8_t lastHour;
    uint8_t lastMinute;
    uint8_t lastSecond;
    uint8_t lastDay;
    uint8_t lastMonth;
    uint16_t lastYear;
    bool lastAlarmEnabled;
    uint8_t lastAlarmHour;
    uint8_t lastAlarmMin;
    float lastFMFreq;
    bool lastWiFiStatus;
    
    // Analog clock center and radius
    int16_t clockCenterX;
    int16_t clockCenterY;
    int16_t clockRadius;
    
    void drawHourHand(uint8_t hour, uint8_t minute, bool erase);
    void drawMinuteHand(uint8_t minute, bool erase);
    void drawSecondHand(uint8_t second, bool erase);

public:
    DisplayILI9341(int8_t cs, int8_t dc, int8_t rst, int8_t mosi, int8_t sck, int8_t miso, int8_t bl);
    
    void begin();
    void clear();
    void setBrightness(uint8_t level);
    uint8_t getBrightness();  // Add getter
    
    // Smart update functions (only redraw if changed)
    void updateTime(uint8_t hour, uint8_t minute, uint8_t second);
    void updateDate(uint16_t year, uint8_t month, uint8_t day);
    void updateAlarmStatus(bool enabled, uint8_t hour, uint8_t minute);
    void updateFMFrequency(float frequency);
    void updateWiFiStatus(bool connected);
    
    // Direct draw functions
    void drawText(int16_t x, int16_t y, const char* text, uint16_t color, uint8_t size = 1);
    void drawTextWithBackground(int16_t x, int16_t y, const char* text, uint16_t fgColor, uint16_t bgColor, uint8_t size = 1);
    
    void drawBitmap(int16_t x, int16_t y, const uint16_t* bitmap, int16_t w, int16_t h);
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void drawCircle(int16_t x, int16_t y, int16_t r, uint16_t color);
    void fillCircle(int16_t x, int16_t y, int16_t r, uint16_t color);
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
    
    int16_t getWidth();
    int16_t getHeight();
    
    void resetCache();
    void drawClockFace();  // Make public so it can be called from main
};

#endif