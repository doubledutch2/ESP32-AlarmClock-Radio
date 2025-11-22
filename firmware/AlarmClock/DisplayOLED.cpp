// ==================== DisplayOLED.cpp ====================
/*
#include "DisplayOLED.h"
#include "config.h"

DisplayOLED::DisplayOLED(uint8_t sdaPin, uint8_t sclPin) {
  Wire.begin(sdaPin, sclPin);
  display = new Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
  brightnessLevel = 3;
}

DisplayOLED::~DisplayOLED() {
  delete display;
}

bool DisplayOLED::begin() {
  if (!display->begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
    return false;
  }
  clear();
  update();
  return true;
}

void DisplayOLED::clear() {
  display->clearDisplay();
}

void DisplayOLED::update() {
  display->display();
}

void DisplayOLED::setBrightness(uint8_t level) {
  brightnessLevel = level;
  uint8_t levels[] = {0x00, 0x10, 0x20, 0x40, 0x60, 0x80};
  if (level < 6) {
    display->ssd1306_command(SSD1306_SETCONTRAST);
    display->ssd1306_command(levels[level]);
    update();
  }
}

void DisplayOLED::drawClock(int hours, int minutes, int seconds) {
  clear();
  
  // Draw clock face
  display->drawCircle(CLOCK_CENTER_X, CLOCK_CENTER_Y, CLOCK_RADIUS, SSD1306_WHITE);
  display->drawCircle(CLOCK_CENTER_X, CLOCK_CENTER_Y, CLOCK_RADIUS - 1, SSD1306_WHITE);
  
  // Draw hour markers
  for (int i = 0; i < 12; i++) {
    float angle = (i * 30 - 90) * PI / 180.0;
    int x1 = CLOCK_CENTER_X + (CLOCK_RADIUS - 4) * cos(angle);
    int y1 = CLOCK_CENTER_Y + (CLOCK_RADIUS - 4) * sin(angle);
    int x2 = CLOCK_CENTER_X + (CLOCK_RADIUS - 1) * cos(angle);
    int y2 = CLOCK_CENTER_Y + (CLOCK_RADIUS - 1) * sin(angle);
    
    if (i % 3 == 0) {
      display->drawLine(x1, y1, x2, y2, SSD1306_WHITE);
    } else {
      display->drawPixel(x2, y2, SSD1306_WHITE);
    }
  }
  
  // Calculate hand angles
  float secondAngle = (seconds * 6 - 90) * PI / 180.0;
  float minuteAngle = (minutes * 6 + seconds * 0.1 - 90) * PI / 180.0;
  float hourAngle = (hours * 30 + minutes * 0.5 - 90) * PI / 180.0;
  
  // Draw hands
  int hourX = CLOCK_CENTER_X + (CLOCK_RADIUS - 14) * cos(hourAngle);
  int hourY = CLOCK_CENTER_Y + (CLOCK_RADIUS - 14) * sin(hourAngle);
  display->drawLine(CLOCK_CENTER_X, CLOCK_CENTER_Y, hourX, hourY, SSD1306_WHITE);
  display->drawLine(CLOCK_CENTER_X + 1, CLOCK_CENTER_Y, hourX + 1, hourY, SSD1306_WHITE);
  
  int minuteX = CLOCK_CENTER_X + (CLOCK_RADIUS - 6) * cos(minuteAngle);
  int minuteY = CLOCK_CENTER_Y + (CLOCK_RADIUS - 6) * sin(minuteAngle);
  display->drawLine(CLOCK_CENTER_X, CLOCK_CENTER_Y, minuteX, minuteY, SSD1306_WHITE);
  display->drawLine(CLOCK_CENTER_X + 1, CLOCK_CENTER_Y, minuteX + 1, minuteY, SSD1306_WHITE);
  
  int secondX = CLOCK_CENTER_X + (CLOCK_RADIUS - 2) * cos(secondAngle);
  int secondY = CLOCK_CENTER_Y + (CLOCK_RADIUS - 2) * sin(secondAngle);
  display->drawLine(CLOCK_CENTER_X, CLOCK_CENTER_Y, secondX, secondY, SSD1306_WHITE);
  
  display->fillCircle(CLOCK_CENTER_X, CLOCK_CENTER_Y, 2, SSD1306_WHITE);
  
  // Digital time
  display->setTextSize(1);
  display->setTextColor(SSD1306_WHITE);
  display->setCursor(70, 56);
  if (hours < 10) display->print("0");
  display->print(hours);
  display->print(":");
  if (minutes < 10) display->print("0");
  display->print(minutes);
  display->print(":");
  if (seconds < 10) display->print("0");
  display->print(seconds);
}

void DisplayOLED::drawInfo(int volume, int brightness) {
  display->setTextSize(1);
  display->setTextColor(SSD1306_WHITE);
  
  display->setCursor(70, 20);
  display->print("BL:");
  display->print(brightness);
  
  display->setCursor(70, 40);
  display->print("Vol:");
  display->print(volume);
}

void DisplayOLED::showStartupMessage(const char* message) {
  clear();
  display->setTextSize(1);
  display->setTextColor(SSD1306_WHITE);
  display->setCursor(20, 28);
  display->println(message);
  update();
}
*/
