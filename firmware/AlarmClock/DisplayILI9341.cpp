#include "DisplayILI9341.h"
#include "Config.h"
#include <math.h>

DisplayILI9341::DisplayILI9341(int8_t cs, int8_t dc, int8_t rst, int8_t mosi, int8_t sck, int8_t miso, int8_t bl)
    : backlightPin(bl), brightness(255) {
    
    // Note: TFT_eSPI uses User_Setup.h for pin configuration
    // The constructor parameters are kept for compatibility but not used
    
    // Initialize cache to impossible values
    lastHour = 255;
    lastMinute = 255;
    lastSecond = 255;
    lastDay = 255;
    lastMonth = 255;
    lastYear = 0;
    lastAlarmEnabled = false;
    lastAlarmHour = 255;
    lastAlarmMin = 255;
    lastFMFreq = -1.0;
    lastWiFiStatus = false;
    lastDateStr = "";
    lastTimeStr = "";
    
    // Analog clock settings (right side of screen)
    // Moved up and right to avoid digital clock
    clockCenterX = 240;
    clockCenterY = 70;
    clockRadius = 55;
}

void DisplayILI9341::begin() {
    tft.init();
    tft.setRotation(1); // Landscape (320x240)
    tft.fillScreen(0x5AEB);

    tft.setCursor(0, 0, 2);
    tft.setTextColor(TFT_WHITE,TFT_BLACK);  
    tft.setTextSize(1);
    tft.println("Hello World!");

    if (backlightPin >= 0) {
        pinMode(backlightPin, OUTPUT);
        ledcAttach(backlightPin, 5000, 8);
        setBrightness(brightness);
    }
    
    clear();
}

void DisplayILI9341::clear() {
    tft.fillScreen(TFT_BLACK);
    resetCache();
}

void DisplayILI9341::resetCache() {
    lastHour = 255;
    lastMinute = 255;
    lastSecond = 255;
    lastDay = 255;
    lastMonth = 255;
    lastYear = 0;
    lastAlarmEnabled = false;
    lastAlarmHour = 255;
    lastAlarmMin = 255;
    lastFMFreq = -1.0;
    lastWiFiStatus = false;
    lastDateStr = "";
    lastTimeStr = "";
}

void DisplayILI9341::setBrightness(uint8_t level) {
    brightness = level;
    if (backlightPin >= 0) {
        ledcWrite(backlightPin, brightness);
    }
}

uint8_t DisplayILI9341::getBrightness() {
    return brightness;
}

// ===== ANALOG CLOCK FUNCTIONS =====
void DisplayILI9341::drawClockFace() {
    if (!ENABLE_DRAW) {
        return;
    }

    // Draw outer circle
    tft.drawCircle(clockCenterX, clockCenterY, clockRadius, TFT_WHITE);
    tft.drawCircle(clockCenterX, clockCenterY, clockRadius - 1, TFT_WHITE);
    
    // Draw hour markers
    for (int i = 0; i < 12; i++) {
        float angle = i * 30 - 90;
        float rad = angle * PI / 180.0;
        
        int x1 = clockCenterX + (clockRadius - 10) * cos(rad);
        int y1 = clockCenterY + (clockRadius - 10) * sin(rad);
        int x2 = clockCenterX + (clockRadius - 5) * cos(rad);
        int y2 = clockCenterY + (clockRadius - 5) * sin(rad);
        
        tft.drawLine(x1, y1, x2, y2, TFT_WHITE);
    }
    
    // Draw center dot
    tft.fillCircle(clockCenterX, clockCenterY, 3, TFT_WHITE);
}

void DisplayILI9341::drawHourHand(uint8_t hour, uint8_t minute, bool erase) {
    if (!ENABLE_DRAW) {
        return;
    }

    float angle = ((hour % 12) * 30 + minute * 0.5) - 90;
    float rad = angle * PI / 180.0;
    int length = clockRadius - 25;
    
    int x = clockCenterX + length * cos(rad);
    int y = clockCenterY + length * sin(rad);
    
    uint16_t color = erase ? TFT_BLACK : TFT_WHITE;
    tft.drawLine(clockCenterX, clockCenterY, x, y, color);
    tft.drawLine(clockCenterX + 1, clockCenterY, x + 1, y, color);
    tft.drawLine(clockCenterX, clockCenterY + 1, x, y + 1, color);
}

void DisplayILI9341::drawMinuteHand(uint8_t minute, bool erase) {
    if (!ENABLE_DRAW) {
        return;
    }

    float angle = (minute * 6) - 90;
    float rad = angle * PI / 180.0;
    int length = clockRadius - 15;
    
    int x = clockCenterX + length * cos(rad);
    int y = clockCenterY + length * sin(rad);
    
    uint16_t color = erase ? TFT_BLACK : TFT_CYAN;
    tft.drawLine(clockCenterX, clockCenterY, x, y, color);
}

void DisplayILI9341::drawSecondHand(uint8_t second, bool erase) {
    if (!ENABLE_DRAW) {
        return;
    }

    float angle = (second * 6) - 90;
    float rad = angle * PI / 180.0;
    int length = clockRadius - 10;
    
    int x = clockCenterX + length * cos(rad);
    int y = clockCenterY + length * sin(rad);
    
    uint16_t color = erase ? TFT_BLACK : TFT_RED;
    tft.drawLine(clockCenterX, clockCenterY, x, y, color);
}

// ===== SMART UPDATE FUNCTIONS =====
void DisplayILI9341::updateTime(uint8_t hour, uint8_t minute, uint8_t second) {
    if (!ENABLE_DRAW) {
        return;
    }

    // Update digital time (only if changed)
    if (hour != lastHour || minute != lastMinute) {
        // FULLY clear the time area to prevent ghosting
        /*
        tft.fillRect(10, 60, 160, 40, TFT_BLACK);
        
        char timeStr[6];
        sprintf(timeStr, "%02d:%02d", hour, minute);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setTextSize(5);
        tft.setCursor(10, 60);
        tft.print(timeStr);
        */
    }
    
    // Update analog clock
    if (lastSecond != 255) {
        drawSecondHand(lastSecond, true);
    }
    if (lastMinute != 255 && lastMinute != minute) {
        drawMinuteHand(lastMinute, true);
    }
    if (lastHour != 255 && (lastHour != hour || lastMinute != minute)) {
        drawHourHand(lastHour, lastMinute, true);
    }

    lastHour = hour;
    lastMinute = minute;
    lastSecond = second;
    
    drawHourHand(hour, minute, false);
    drawMinuteHand(minute, false);
    drawSecondHand(second, false);
    
    tft.fillCircle(clockCenterX, clockCenterY, 3, TFT_WHITE);
}

void DisplayILI9341::updateDate(uint16_t year, uint8_t month, uint8_t day) {
    if (!ENABLE_DRAW) {
        return;
    }

    if (year != lastYear || month != lastMonth || day != lastDay) {
        // FULLY clear the date area to prevent ghosting
/*
        tft.fillRect(10, 110, 200, 20, TFT_BLACK);      
        char dateStr[15];
        sprintf(dateStr, "%04d-%02d-%02d", year, month, day);
        tft.setCursor(10, 70);
        tft.setTextColor(TFT_CYAN, TFT_BLACK);
        tft.setTextSize(2);
        tft.print(dateStr);
*/        
        lastYear = year;
        lastMonth = month;
        lastDay = day;
    }
}

void DisplayILI9341::updateDateFormatted(const String& dateStr, const String& timeStr) {
    if (!ENABLE_DRAW) {
        return;
    }
    // Check if date changed - MOVED to bottom, above SETUP button
    if (dateStr != lastDateStr) {
        // FULLY clear the date area (bottom of screen, Y: dateStrRow)
        tft.fillRect(startColumn, dateStrRow, dateWidth,dateHeight, TFT_BLACK);
        tft.setCursor(startColumn, dateStrRow);
        tft.setTextColor(TFT_CYAN, TFT_BLACK);
        tft.setTextSize(2);
        tft.print(dateStr);
        
        lastDateStr = dateStr;
    }
    
    // Check if time changed (HH:MM only, ignore seconds) - SMALLER and HIGHER
    String shortTime = timeStr.substring(0, 5);  // Get "HH:MM"
    String lastShortTime = lastTimeStr.substring(0, 5);
    
    if (shortTime != lastShortTime || lastTimeStr.length() == 0) {
        // FULLY clear the time area (top left, smaller size 4)
        tft.fillRect(startColumn, clockRow, clockWidth, clockHeight, TFT_BLACK);
        tft.setCursor(startColumn, clockRow);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setTextSize(4);  // Reduced from 5 to 4
        tft.print(shortTime);
        
        lastTimeStr = timeStr;
    }
}

void DisplayILI9341::updateAlarmStatus(bool enabled, uint8_t hour, uint8_t minute) {
    if (!ENABLE_DRAW) {
        return;
    }

    if (enabled != lastAlarmEnabled || hour != lastAlarmHour || minute != lastAlarmMin) {
        // FULLY clear the alarm area (below digital clock)
/*

        This messes up the clock display
*/        
        tft.fillRect(startColumn, clockRow + clockHeight, alarmWidth, alarmHeight, TFT_BLACK);

        // Only show alarm if enabled
        if (enabled) {
            char alarmStr[20];
            sprintf(alarmStr, "ALARM: %02d:%02d", hour, minute);
            tft.setTextColor(TFT_GREEN, TFT_BLACK);
            tft.setCursor(10, 80);
            tft.setTextSize(2);
            tft.print(alarmStr);
        }
        
        lastAlarmEnabled = enabled;
        lastAlarmHour = hour;
        lastAlarmMin = minute;
    }
}

void DisplayILI9341::updateFMFrequency(float frequency) {
    if (!ENABLE_DRAW) {
        return;
    }

    if (abs(frequency - lastFMFreq) > 0.05) {
        // FULLY clear the FM area (below alarm)
        tft.fillRect(10, 110, 150, 20, TFT_BLACK);
        
        char freqStr[15];
        sprintf(freqStr, "FM: %.1f MHz", frequency);
        tft.setCursor(10, 110);
        tft.setTextColor(TFT_YELLOW, TFT_BLACK);
        tft.setTextSize(2);
        tft.print(freqStr);
        
        lastFMFreq = frequency;
    }
}

void DisplayILI9341::updateWiFiStatus(bool connected) {
    if (!ENABLE_DRAW) {
        return;
    }

    if (connected != lastWiFiStatus) {
        // FULLY clear the WiFi area
        tft.fillRect(5, 5, 70, 15, TFT_BLACK);
        
        tft.setCursor(5, 5);
        tft.setTextSize(1);
        if (connected) {
            tft.setTextColor(TFT_GREEN, TFT_BLACK);
            tft.print("WiFi OK");
        } else {
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.print("WiFi OFF");
        }
        
        lastWiFiStatus = connected;
    }
}

// ===== DIRECT DRAW FUNCTIONS =====
void DisplayILI9341::drawText(int16_t x, int16_t y, const char* text, uint16_t color, uint8_t size) {
    tft.setCursor(x, y, 2);
    tft.setTextColor(color, TFT_BLACK);
    tft.setTextSize(size);
    tft.println(text);
}

void DisplayILI9341::drawTextWithBackground(int16_t x, int16_t y, const char* text, uint16_t fgColor, uint16_t bgColor, uint8_t size) {
    tft.setCursor(x, y);
    tft.setTextColor(fgColor, bgColor);
    tft.setTextSize(size);
    tft.print(text);
}

void DisplayILI9341::drawBitmap(int16_t x, int16_t y, const uint16_t* bitmap, int16_t w, int16_t h) {
    if (!ENABLE_DRAW) {
        return;
    }
    tft.pushImage(x, y, w, h, bitmap);
}

/*
x = startCol
y = startRow
w = width
h = height
*/
void DisplayILI9341::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (!ENABLE_DRAW) {
        return;
    }
    tft.fillRect(x, y, w, h, color);
}

void DisplayILI9341::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (!ENABLE_DRAW) {
        return;
    }
    tft.drawRect(x, y, w, h, color);
}

void DisplayILI9341::drawCircle(int16_t x, int16_t y, int16_t r, uint16_t color) {
    if (!ENABLE_DRAW) {
        return;
    }
    tft.drawCircle(x, y, r, color);
}

void DisplayILI9341::fillCircle(int16_t x, int16_t y, int16_t r, uint16_t color) {
    if (!ENABLE_DRAW) {
        return;
    }
    tft.fillCircle(x, y, r, color);
}

void DisplayILI9341::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
    if (!ENABLE_DRAW) {
        return;
    }
    tft.drawLine(x0, y0, x1, y1, color);
}

int16_t DisplayILI9341::getWidth() {
    return tft.width();
}

int16_t DisplayILI9341::getHeight() {
    return tft.height();
}