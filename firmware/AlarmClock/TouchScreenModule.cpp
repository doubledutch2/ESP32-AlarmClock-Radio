#include "TouchScreenModule.h"

TouchScreenModule::TouchScreenModule(int cs, int irq) 
    : touchCS(cs), touchIRQ(irq), initialized(false), lastTouchTime(0) {
    ts = new XPT2046_Touchscreen(cs, irq);
}

bool TouchScreenModule::begin() {
    if (!ts) {
        Serial.println("TouchScreen: ts object is null");
        return false;
    }
    
    if (initialized) {
        Serial.println("TouchScreen: Already initialized");
        return true;
    }
    
    if (INIT_TOUCHSCREEN_FIRST) {
        initialized = true;
        return true;
    }

    Serial.println("TouchScreen: Starting initialization NOW...");
    
    try {
        Serial.println("TouchScreen: Calling ts->begin()...");
        ts->begin();
        Serial.println("TouchScreen: ts->begin() completed successfully");
        initialized = true;
        return true;
    } catch (...) {
        Serial.println("TouchScreen: Exception during begin()");
        return false;
    }
}

bool TouchScreenModule::ensureInitialized() {
    if (initialized) return true;
    
    // Try to initialize on first use
    Serial.println("TouchScreen: Lazy initialization triggered");
    return begin();
}

bool TouchScreenModule::isTouched() {
    if (!ts) return false;
    
    // Initialize on first touch attempt
    if (!ensureInitialized()) {
        Serial.println("TouchScreen: Failed to initialize, disabling");
        return false;
    }
    
    // Check debounce
    unsigned long now = millis();
    if (now - lastTouchTime < DEBOUNCE_DELAY) {
    Serial.println("TouchScreenModule: isTouched3");
        return false;
    }
    
    if (ts->touched()) {
        Serial.println("TouchScreenModule: isTouched");
        lastTouchTime = now;
        return true;
    }

    return false;
}

TouchPoint TouchScreenModule::getPoint() {
    Serial.println("TouchScreenModule: getPoint");
    TouchPoint result = {0, 0, 0, false};
    
    if (!ts) return result;
    
    // Ensure initialized
    if (!ensureInitialized()) return result;
    
    if (!ts->touched()) {
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
    Serial.println("TouchScreenModule: mapAndInvertPoint");
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