#include "TouchScreenModule.h"

TouchScreenModule::TouchScreenModule(int cs, int irq) 
    : touchCS(cs), touchIRQ(irq), lastTouchTime(0) {
    ts = new XPT2046_Touchscreen(cs, irq);
}

bool TouchScreenModule::begin() {
    if (!ts) return false;
    
    // XPT2046 begin() can hang if hardware isn't connected properly
    // Try to initialize but don't let it block forever
    Serial.println("TouchScreen: Attempting initialization...");
    
    // The XPT2046 library doesn't have a timeout, so we just try it
    // If your hardware isn't connected, you may need to comment this out
    ts->begin();
    
    // Always return true - we'll check if it actually works when we try to use it
    Serial.println("TouchScreen: Initialization attempted");
    return true;
}

bool TouchScreenModule::isTouched() {
    if (!ts) return false;
    
    // Check debounce
    unsigned long now = millis();
    if (now - lastTouchTime < DEBOUNCE_DELAY) {
        return false;
    }
    
    if (ts->touched()) {
        lastTouchTime = now;
        return true;
    }
    
    return false;
}

TouchPoint TouchScreenModule::getPoint() {
    TouchPoint result = {0, 0, 0, false};
    
    if (!ts || !ts->touched()) {
        return result;
    }
    
    TS_Point raw = ts->getPoint();
    
    // Check pressure threshold
    if (raw.z < TOUCH_PRESSURE_THRESHOLD) {
        return result;
    }
    
    // Filter out spurious readings (z=4095 with x,y near 0 means floating/no touch)
    if (raw.z >= 4090 && raw.x < 50 && raw.y < 50) {
        return result;
    }
    
    return mapAndInvertPoint(raw);
}

TouchPoint TouchScreenModule::mapAndInvertPoint(TS_Point raw) {
    TouchPoint mapped;
    
    // Map raw coordinates to screen coordinates
    int sx = map(raw.x, RAW_X_MIN, RAW_X_MAX, 0, SCREEN_WIDTH);
    int sy = map(raw.y, RAW_Y_MIN, RAW_Y_MAX, 0, SCREEN_HEIGHT);
    
    // Apply inversion/flip (from your calibration)
    sx = SCREEN_WIDTH - sx;
    sy = SCREEN_HEIGHT - sy;
    
    // Constrain to screen bounds
    mapped.x = constrain(sx, 0, SCREEN_WIDTH - 1);
    mapped.y = constrain(sy, 0, SCREEN_HEIGHT - 1);
    mapped.z = raw.z;
    mapped.valid = true;
    
    // Debug output
    Serial.printf("Touch: raw(%d,%d) -> screen(%d,%d) z=%d\n", 
                  raw.x, raw.y, mapped.x, mapped.y, mapped.z);
    
    return mapped;
}

bool TouchScreenModule::isTouchInArea(int x, int y, int w, int h) {
    TouchPoint p = getPoint();
    
    if (!p.valid) return false;
    
    bool inArea = (p.x >= x && p.x <= x + w && p.y >= y && p.y <= y + h);
    
    if (inArea) {
        Serial.printf("Touch in area: (%d,%d) in [%d,%d,%d,%d]\n", 
                     p.x, p.y, x, y, w, h);
    }
    
    return inArea;
}