#ifndef TOUCHSCREEN_MODULE_H
#define TOUCHSCREEN_MODULE_H

#include <XPT2046_Touchscreen.h>

// Touch calibration values (from your working example)
#define RAW_X_MIN 300
#define RAW_X_MAX 3800
#define RAW_Y_MIN 270
#define RAW_Y_MAX 3700

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

// Touch pressure threshold
#define TOUCH_PRESSURE_THRESHOLD 5

struct TouchPoint {
    int x;
    int y;
    int z;
    bool valid;
};

class TouchScreenModule {
private:
    XPT2046_Touchscreen* ts;
    int touchCS;
    int touchIRQ;
    
    // Debouncing
    unsigned long lastTouchTime;
    static const unsigned long DEBOUNCE_DELAY = 200;

public:
    TouchScreenModule(int cs, int irq);
    
    bool begin();
    bool isTouched();
    TouchPoint getPoint();
    
    // Helper to check if touch is within a rectangular area
    bool isTouchInArea(int x, int y, int w, int h);
    
private:
    TouchPoint mapAndInvertPoint(TS_Point raw);
};

#endif
