#ifndef TOUCHSCREEN_MODULE_H
#define TOUCHSCREEN_MODULE_H

#include <TFT_eSPI.h>

// Touch calibration values (from your working example)
#define RAW_X_MIN 300
#define RAW_X_MAX 3800
#define RAW_Y_MIN 270
#define RAW_Y_MAX 3700

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

// Touch pressure threshold
#define TOUCH_PRESSURE_THRESHOLD 600  // TFT_eSPI uses different scale

struct TouchPoint {
    int x;
    int y;
    int z;
    bool valid;
};

class TouchScreenModule {
private:
    TFT_eSPI* tft;  // Use TFT_eSPI instead of XPT2046_Touchscreen
    bool initialized;
    
    // Debouncing
    unsigned long lastTouchTime;
    static const unsigned long DEBOUNCE_DELAY = 200;

public:
    TouchScreenModule(TFT_eSPI* tftPtr);  // Changed constructor - takes TFT_eSPI pointer
    
    bool begin();  // Simplified - TFT_eSPI handles initialization
    bool isTouched();
    TouchPoint getPoint();
    
    // Helper to check if touch is within a rectangular area
    bool isTouchInArea(int x, int y, int w, int h);
    
private:
    TouchPoint mapAndInvertPoint(uint16_t rawX, uint16_t rawY, uint16_t rawZ);
};

#endif