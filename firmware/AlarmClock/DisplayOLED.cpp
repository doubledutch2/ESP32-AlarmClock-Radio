#include "DisplayOLED.h"
#include <math.h>

DisplayOLED::DisplayOLED(int sda, int scl) 
    : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET),
      sdaPin(sda), sclPin(scl), currentMode(MODE_CLOCK),
      brightness(128), initialized(false) {
}

bool DisplayOLED::begin() {
    Wire.begin(sdaPin, sclPin);
    
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("OLED: Failed to initialize");
        return false;
    }
    
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.display();
    
    initialized = true;
    Serial.println("OLED: Initialized successfully");
    return true;
}

void DisplayOLED::clear() {
    if (!initialized) return;
    display.clearDisplay();
}

void DisplayOLED::update() {
    if (!initialized) return;
    display.display();
}

void DisplayOLED::setMode(DisplayMode mode) {
    currentMode = mode;
    clear();
}

DisplayMode DisplayOLED::getMode() {
    return currentMode;
}

void DisplayOLED::setBrightness(uint8_t level) {
    brightness = level;
    if (initialized) {
        display.ssd1306_command(SSD1306_SETCONTRAST);
        display.ssd1306_command(level);
    }
}

uint8_t DisplayOLED::getBrightness() {
    return brightness;
}

void DisplayOLED::drawClock(uint8_t hour, uint8_t minute, uint8_t second) {
    if (!initialized) return;
    
    clear();
    
    if (currentMode == MODE_CLOCK) {
        // Draw analog clock on left, digital on right
        drawClockAnalog(hour, minute, second);
        drawClockDigital(hour, minute, second);
    } else {
        drawClockDigital(hour, minute, second);
    }
    
    update();
}

void DisplayOLED::drawClockAnalog(uint8_t hour, uint8_t minute, uint8_t second) {
    int centerX = 32;
    int centerY = 32;
    int radius = 28;
    
    // Draw clock circle
    display.drawCircle(centerX, centerY, radius, SSD1306_WHITE);
    
    // Draw hour markers
    for (int i = 0; i < 12; i++) {
        float angle = (i * 30 - 90) * PI / 180.0;
        int x1 = centerX + (radius - 4) * cos(angle);
        int y1 = centerY + (radius - 4) * sin(angle);
        int x2 = centerX + (radius - 2) * cos(angle);
        int y2 = centerY + (radius - 2) * sin(angle);
        display.drawLine(x1, y1, x2, y2, SSD1306_WHITE);
    }
    
    // Draw hour hand
    float hourAngle = ((hour % 12) * 30 + minute * 0.5 - 90) * PI / 180.0;
    int hourX = centerX + (radius - 12) * cos(hourAngle);
    int hourY = centerY + (radius - 12) * sin(hourAngle);
    display.drawLine(centerX, centerY, hourX, hourY, SSD1306_WHITE);
    
    // Draw minute hand
    float minAngle = (minute * 6 - 90) * PI / 180.0;
    int minX = centerX + (radius - 6) * cos(minAngle);
    int minY = centerY + (radius - 6) * sin(minAngle);
    display.drawLine(centerX, centerY, minX, minY, SSD1306_WHITE);
    
    // Draw center dot
    display.fillCircle(centerX, centerY, 2, SSD1306_WHITE);
}

void DisplayOLED::drawClockDigital(uint8_t hour, uint8_t minute, uint8_t second) {
    char timeStr[9];
    sprintf(timeStr, "%02d:%02d:%02d", hour, minute, second);
    
    display.setTextSize(2);
    display.setCursor(64, 24);
    display.print(timeStr);
}

void DisplayOLED::drawDate(uint16_t year, uint8_t month, uint8_t day) {
    if (!initialized) return;
    
    char dateStr[11];
    sprintf(dateStr, "%04d-%02d-%02d", year, month, day);
    
    display.setTextSize(1);
    display.setCursor(64, 45);
    display.print(dateStr);
}

void DisplayOLED::showInternetRadio(const char* stationName, const char* artist, const char* title) {
    if (!initialized) return;
    
    clear();
    
    display.setTextSize(1);
    
    // Station name at top
    display.setCursor(0, 0);
    display.print("Internet Radio");
    
    display.setCursor(0, 12);
    display.setTextSize(2);
    display.print(stationName);
    
    // Artist and title
    if (artist && strlen(artist) > 0) {
        display.setTextSize(1);
        display.setCursor(0, 40);
        display.print(artist);
    }
    
    if (title && strlen(title) > 0) {
        display.setCursor(0, 52);
        display.print(title);
    }
    
    update();
}

void DisplayOLED::showFMRadio(const char* frequency, const char* rdsStation, const char* rdsText) {
    if (!initialized) return;
    
    clear();
    
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("FM Radio");
    
    // Frequency
    display.setTextSize(3);
    display.setCursor(0, 16);
    display.print(frequency);
    
    // RDS info
    if (rdsStation && strlen(rdsStation) > 0) {
        display.setTextSize(1);
        display.setCursor(0, 48);
        display.print(rdsStation);
    }
    
    if (rdsText && strlen(rdsText) > 0) {
        display.setCursor(0, 56);
        display.print(rdsText);
    }
    
    update();
}

void DisplayOLED::drawInfo(int volume, int brightness) {
    if (!initialized) return;
    
    char info[20];
    sprintf(info, "V:%d B:%d", volume, brightness);
    
    display.setTextSize(1);
    display.setCursor(0, 56);
    display.print(info);
}

void DisplayOLED::showStartupMessage(const char* message) {
    if (!initialized) return;
    
    clear();
    
    display.setTextSize(2);
    display.setCursor(10, 24);
    display.print(message);
    
    update();
    delay(2000);
}

void DisplayOLED::showError(const char* message) {
    if (!initialized) return;
    
    clear();
    
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("ERROR:");
    
    display.setCursor(0, 16);
    display.print(message);
    
    update();
}

void DisplayOLED::drawText(int16_t x, int16_t y, const char* text, uint16_t color, uint8_t size) {
    if (!initialized) return;
    
    display.setTextSize(size);
    display.setCursor(x, y);
    display.print(text);
}
