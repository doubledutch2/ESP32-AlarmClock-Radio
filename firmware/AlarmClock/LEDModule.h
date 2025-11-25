#ifndef LED_MODULE_H
#define LED_MODULE_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

class LEDModule {
public:
    // Color constants
    static const uint32_t COLOR_RED = 0xFF0000;
    static const uint32_t COLOR_GREEN = 0x00FF00;
    static const uint32_t COLOR_BLUE = 0x0000FF;
    static const uint32_t COLOR_YELLOW = 0xFFFF00;
    static const uint32_t COLOR_CYAN = 0x00FFFF;
    static const uint32_t COLOR_MAGENTA = 0xFF00FF;
    static const uint32_t COLOR_WHITE = 0xFFFFFF;
    static const uint32_t COLOR_OFF = 0x000000;
    
private:
    Adafruit_NeoPixel pixel;
    uint8_t ledPin;
    uint8_t brightness;
    uint32_t currentColor;

public:
    LEDModule(uint8_t pin);
    
    void begin();
    void setColor(uint32_t color, uint8_t brightness = 255);
    void setRGB(uint8_t r, uint8_t g, uint8_t b, uint8_t brightness = 255);
    void setBrightness(uint8_t brightness);
    void off();
    void pulse(uint32_t color, int duration = 1000);
    
    uint32_t getCurrentColor();
    uint8_t getBrightness();
};

#endif