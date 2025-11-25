#ifndef DISPLAY_INTERFACE_H
#define DISPLAY_INTERFACE_H

#include <Arduino.h>

// Display modes
enum DisplayMode {
    MODE_CLOCK,
    MODE_INTERNET_RADIO,
    MODE_FM_RADIO,
    MODE_MENU
};

// Abstract base class for all displays
class DisplayInterface {
public:
    virtual ~DisplayInterface() {}
    
    // Initialization
    virtual bool begin() = 0;
    virtual void clear() = 0;
    virtual void update() = 0;
    
    // Mode management
    virtual void setMode(DisplayMode mode) = 0;
    virtual DisplayMode getMode() = 0;
    
    // Brightness control
    virtual void setBrightness(uint8_t level) = 0;
    virtual uint8_t getBrightness() = 0;
    
    // Clock display
    virtual void drawClock(uint8_t hour, uint8_t minute, uint8_t second) = 0;
    virtual void drawDate(uint16_t year, uint8_t month, uint8_t day) = 0;
    
    // Internet Radio display
    virtual void showInternetRadio(const char* stationName, const char* artist, const char* title) = 0;
    
    // FM Radio display
    virtual void showFMRadio(const char* frequency, const char* rdsStation, const char* rdsText) = 0;
    
    // Status/info display
    virtual void drawInfo(int volume, int brightness) = 0;
    virtual void showStartupMessage(const char* message) = 0;
    virtual void showError(const char* message) = 0;
    
    // Text display
    virtual void drawText(int16_t x, int16_t y, const char* text, uint16_t color, uint8_t size = 1) = 0;
};

#endif