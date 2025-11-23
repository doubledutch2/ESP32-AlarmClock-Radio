#include "DisplayILI9341.h"

DisplayILI9341::DisplayILI9341(int8_t cs, int8_t dc, int8_t rst, int8_t mosi, int8_t miso, int8_t sclk, int8_t bl)
    : tft(cs,dc,mosi,sclk,rst,miso), backlightPin(bl),brightness(255) {}
// Adafruit_ILI9341(int8_t _CS, int8_t _DC, int8_t _MOSI, int8_t _SCLK, int8_t _RST = -1, int8_t _MISO = -1)

/*
DisplayILI9341::DisplayILI9341(int8_t cs, int8_t dc, int8_t rst, int8_t bl)
    : tft(cs, dc, rst), backlightPin(bl), brightness(255) {}
*/    
void DisplayILI9341::begin() {
    tft.begin();
    tft.setRotation(1); // Landscape
    
    if (backlightPin >= 0) {
        pinMode(backlightPin, OUTPUT);
        // ESP32 Core 3.x: ledcAttach instead of ledcSetup + ledcAttachPin
        ledcAttach(backlightPin, 5000, 8); // pin, frequency, resolution (8-bit = 0-255)
        setBrightness(brightness);
    }
    
    clear();
}

void DisplayILI9341::clear() {
    tft.fillScreen(ILI9341_BLACK);
}

void DisplayILI9341::setBrightness(uint8_t level) {
    brightness = level;
    if (backlightPin >= 0) {
        ledcWrite(backlightPin, brightness);
    }
}

void DisplayILI9341::drawText(int16_t x, int16_t y, const char* text, uint16_t color, uint8_t size) {
    tft.setCursor(x, y);
    tft.setTextColor(color);
    tft.setTextSize(size);
    tft.print(text);
}

void DisplayILI9341::drawTime(int16_t x, int16_t y, uint8_t hour, uint8_t minute, uint8_t second) {
    char timeStr[12];
    sprintf(timeStr, "%02d:%02d:%02d", hour, minute, second);
    
    tft.setTextSize(4);
    tft.setCursor(x, y);
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    tft.print(timeStr);
}

void DisplayILI9341::drawDate(int16_t x, int16_t y, uint16_t year, uint8_t month, uint8_t day) {
    char dateStr[16];
    sprintf(dateStr, "%04d-%02d-%02d", year, month, day);
    
    tft.setTextSize(2);
    tft.setCursor(x, y);
    tft.setTextColor(ILI9341_CYAN, ILI9341_BLACK);
    tft.print(dateStr);
}

void DisplayILI9341::drawAlarmStatus(int16_t x, int16_t y, bool enabled, uint8_t hour, uint8_t minute) {
    char alarmStr[20];
    if (enabled) {
        sprintf(alarmStr, "ALARM: %02d:%02d", hour, minute);
        tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
    } else {
        sprintf(alarmStr, "ALARM: OFF");
        tft.setTextColor(ILI9341_RED, ILI9341_BLACK);
    }
    
    tft.setTextSize(2);
    tft.setCursor(x, y);
    tft.print(alarmStr);
}

void DisplayILI9341::drawFMFrequency(int16_t x, int16_t y, float frequency) {
    char freqStr[12];
    sprintf(freqStr, "FM: %.1f", frequency);
    
    tft.setTextSize(2);
    tft.setCursor(x, y);
    tft.setTextColor(ILI9341_YELLOW, ILI9341_BLACK);
    tft.print(freqStr);
}

void DisplayILI9341::drawBitmap(int16_t x, int16_t y, const uint16_t* bitmap, int16_t w, int16_t h) {
    tft.drawRGBBitmap(x, y, bitmap, w, h);
}

void DisplayILI9341::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    tft.fillRect(x, y, w, h, color);
}

void DisplayILI9341::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    tft.drawRect(x, y, w, h, color);
}

int16_t DisplayILI9341::getWidth() {
    return tft.width();
}

int16_t DisplayILI9341::getHeight() {
    return tft.height();
}