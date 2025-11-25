#include "LEDModule.h"

LEDModule::LEDModule(uint8_t pin) 
    : pixel(1, pin, NEO_GRB + NEO_KHZ800), ledPin(pin), brightness(255), currentColor(COLOR_OFF) {
}

void LEDModule::begin() {
    pixel.begin();
    pixel.setBrightness(brightness);
    pixel.show(); // Initialize all pixels to 'off'
}

void LEDModule::setColor(uint32_t color, uint8_t brightness) {
    this->brightness = brightness;
    this->currentColor = color;
    pixel.setBrightness(brightness);
    pixel.setPixelColor(0, color);
    pixel.show();
}

void LEDModule::setRGB(uint8_t r, uint8_t g, uint8_t b, uint8_t brightness) {
    uint32_t color = pixel.Color(r, g, b);
    setColor(color, brightness);
}

void LEDModule::setBrightness(uint8_t brightness) {
    this->brightness = brightness;
    pixel.setBrightness(brightness);
    pixel.setPixelColor(0, currentColor);
    pixel.show();
}

void LEDModule::off() {
    setColor(COLOR_OFF, 0);
}

void LEDModule::pulse(uint32_t color, int duration) {
    int steps = 50;
    int delayTime = duration / (steps * 2);
    
    // Fade in
    for (int i = 0; i <= steps; i++) {
        uint8_t b = (brightness * i) / steps;
        pixel.setBrightness(b);
        pixel.setPixelColor(0, color);
        pixel.show();
        delay(delayTime);
    }
    
    // Fade out
    for (int i = steps; i >= 0; i--) {
        uint8_t b = (brightness * i) / steps;
        pixel.setBrightness(b);
        pixel.setPixelColor(0, color);
        pixel.show();
        delay(delayTime);
    }
    
    // Restore original state
    pixel.setBrightness(brightness);
    pixel.setPixelColor(0, currentColor);
    pixel.show();
}

uint32_t LEDModule::getCurrentColor() {
    return currentColor;
}

uint8_t LEDModule::getBrightness() {
    return brightness;
}