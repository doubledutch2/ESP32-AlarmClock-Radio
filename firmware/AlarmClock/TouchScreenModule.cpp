#include "TouchScreenModule.h"

TouchScreenModule::TouchScreenModule(TFT_eSPI* tftPtr) 
    : tft(tftPtr), initialized(false), lastTouchTime(0) {
}

bool TouchScreenModule::begin() {
    if (!tft) {
        Serial.println("TouchScreen: TFT pointer is null");
        return false;
    }
    
    if (initialized) {
        Serial.println("TouchScreen: Already initialized");
        return true;
    }
    
    Serial.println("TouchScreen: Using TFT_eSPI built-in touch support");
    
    // TFT_eSPI automatically initializes touch when TOUCH_CS is defined in User_Setup.h
    // No additional initialization needed!
    
    initialized = true;
    return true;
}

bool TouchScreenModule::isTouched() {
    if (!tft || !initialized) return false;
    
    // Check debounce
    unsigned long now = millis();
    if (now - lastTouchTime < DEBOUNCE_DELAY) {
        return false;
    }
    
    uint16_t x, y;
    bool touched = tft->getTouch(&x, &y);  // TFT_eSPI's built-in touch function
    
    if (touched) {
        lastTouchTime = now;
        return true;
    }
    
    return false;
}

TouchPoint TouchScreenModule::getPoint() {
    TouchPoint result = {0, 0, 0, false};
    
    if (!tft || !initialized) return result;
    
    uint16_t rawX, rawY;
    
    // Get raw touch coordinates from TFT_eSPI
    bool touched = tft->getTouchRaw(&rawX, &rawY);
    
    if (!touched) {
        return result;
    }
    
    // TFT_eSPI doesn't provide pressure, so we use a dummy value
    uint16_t rawZ = 1000;  // Simulated pressure
    
    // Filter out spurious readings
    if (rawX < 50 && rawY < 50) {
        return result;
    }
    
    return mapAndInvertPoint(rawX, rawY, rawZ);
}

TouchPoint TouchScreenModule::mapAndInvertPoint(uint16_t rawX, uint16_t rawY, uint16_t rawZ) {
    TouchPoint mapped;
    
    // Map raw coordinates to screen coordinates
    int sx = map(rawX, RAW_X_MIN, RAW_X_MAX, 0, SCREEN_WIDTH);
    int sy = map(rawY, RAW_Y_MIN, RAW_Y_MAX, 0, SCREEN_HEIGHT);
    
    // Apply inversion/flip (from your calibration)
    sx = SCREEN_WIDTH - sx;
    sy = SCREEN_HEIGHT - sy;
    
    // Constrain to screen bounds
    mapped.x = constrain(sx, 0, SCREEN_WIDTH - 1);
    mapped.y = constrain(sy, 0, SCREEN_HEIGHT - 1);
    mapped.z = rawZ;
    mapped.valid = true;
    
    // Debug output
    Serial.printf("Touch: raw(%d,%d) -> screen(%d,%d) z=%d\n", 
                  rawX, rawY, mapped.x, mapped.y, mapped.z);
    
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