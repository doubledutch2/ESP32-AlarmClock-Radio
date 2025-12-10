#include "TouchScreenModule.h"

TouchScreenModule::TouchScreenModule(int cs, int irq) 
    : touchCS(cs), touchIRQ(irq), lastTouchTime(0) {
    ts = new XPT2046_Touchscreen(cs, irq);
}

bool TouchScreenModule::begin() {
    if (!ts) return false;
    
    bool success = ts->begin();
    if (success) {
        Serial.println("TouchScreen initialized successfully");
    } else {
        Serial.println("TouchScreen initialization failed");
    }
    return success;
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
