// ==================== DisplayILI9341.cpp ====================
#include "DisplayILI9341.h"
#include "config.h"

DisplayILI9341::DisplayILI9341(uint8_t backlightPin) {
  tft = new TFT_eSPI();
  this->backlightPin = backlightPin;
  brightnessLevel = 5;
  currentMode = MODE_CLOCK;
  
  lastHour = -1;
  lastMinute = -1;
  
  COLOR_BG = DAY_BG;
  COLOR_TEXT = DAY_TEXT;
  COLOR_CLOCK_FACE = DAY_TEXT;
  COLOR_CLOCK_HANDS = DAY_TEXT;
  COLOR_ACCENT = DAY_ACCENT;
}

DisplayILI9341::~DisplayILI9341() {
  delete tft;
}

bool DisplayILI9341::begin() {
  Serial.println("Initializing 2.8\" ILI9341 TFT (240x320)...");
  
  tft->init();
  tft->setRotation(0);  // Portrait (240 wide x 320 tall)
  tft->fillScreen(COLOR_BG);
  
  if (backlightPin > 0) {
    pinMode(backlightPin, OUTPUT);
    ledcSetup(0, 5000, 8);
    ledcAttachPin(backlightPin, 0);
    setBrightness(brightnessLevel);
  }
  
  Serial.printf("TFT initialized: %dx%d\n", tft->width(), tft->height());
  return true;
}

void DisplayILI9341::clear() {
  tft->fillScreen(COLOR_BG);
  lastHour = -1;
  lastMinute = -1;
  lastInfo1 = "";
  lastInfo2 = "";
  lastInfo3 = "";
}

void DisplayILI9341::update() {
  // No buffer to flush
}

void DisplayILI9341::setBrightness(uint8_t level) {
  brightnessLevel = level;
  
  if (backlightPin > 0) {
    uint8_t pwmValues[] = {10, 30, 60, 120, 200, 255};
    uint8_t pwm = (level < 6) ? pwmValues[level] : 255;
    ledcWrite(0, pwm);
  }
  
  if (level <= 1) {
    setNightMode(true);
  } else if (level >= 4) {
    setNightMode(false);
  }
}

void DisplayILI9341::setNightMode(bool night) {
  if (night) {
    COLOR_BG = NIGHT_BG;
    COLOR_TEXT = NIGHT_TEXT;
    COLOR_CLOCK_FACE = NIGHT_TEXT;
    COLOR_CLOCK_HANDS = NIGHT_TEXT;
    COLOR_ACCENT = NIGHT_ACCENT;
  } else {
    COLOR_BG = DAY_BG;
    COLOR_TEXT = DAY_TEXT;
    COLOR_CLOCK_FACE = DAY_TEXT;
    COLOR_CLOCK_HANDS = DAY_TEXT;
    COLOR_ACCENT = DAY_ACCENT;
  }
  clear();
}

void DisplayILI9341::enableNightMode(bool enable) {
  setNightMode(enable);
}

void DisplayILI9341::setMode(DisplayMode mode) {
  if (currentMode != mode) {
    currentMode = mode;
    clear();
  }
}

void DisplayILI9341::drawClock(int hours, int minutes, int seconds) {
  if (currentMode != MODE_CLOCK) {
    setMode(MODE_CLOCK);
  }
  
  bool minuteChanged = (minutes != lastMinute);
  if (minuteChanged || lastHour == -1) {
    drawAnalogClock(hours, minutes, true);
    drawLargeDigitalTime(hours, minutes);
    lastHour = hours;
    lastMinute = minutes;
  }
}

void DisplayILI9341::drawAnalogClock(int hours, int minutes, bool forceRedraw) {
  int centerX = SCREEN_WIDTH / 2;
  int centerY = 110;  // Upper portion
  int radius = 80;
  
  if (forceRedraw) {
    tft->fillCircle(centerX, centerY, radius + 5, COLOR_BG);
    tft->drawCircle(centerX, centerY, radius, COLOR_CLOCK_FACE);
    tft->drawCircle(centerX, centerY, radius - 1, COLOR_CLOCK_FACE);
    
    // Hour markers at 12, 3, 6, 9
    for (int i = 0; i < 12; i += 3) {
      float angle = (i * 30 - 90) * PI / 180.0;
      int x = centerX + (radius - 8) * cos(angle);
      int y = centerY + (radius - 8) * sin(angle);
      tft->fillCircle(x, y, 3, COLOR_CLOCK_FACE);
    }
  }
  
  // Erase old hands
  if (lastMinute >= 0) {
    float oldMinuteAngle = (lastMinute * 6 - 90) * PI / 180.0;
    float oldHourAngle = ((lastHour % 12) * 30 + lastMinute * 0.5 - 90) * PI / 180.0;
    
    int oldMinuteX = centerX + (radius - 12) * cos(oldMinuteAngle);
    int oldMinuteY = centerY + (radius - 12) * sin(oldMinuteAngle);
    int oldHourX = centerX + (radius - 30) * cos(oldHourAngle);
    int oldHourY = centerY + (radius - 30) * sin(oldHourAngle);
    
    tft->drawLine(centerX, centerY, oldMinuteX, oldMinuteY, COLOR_BG);
    tft->drawLine(centerX, centerY, oldHourX, oldHourY, COLOR_BG);
  }
  
  // Draw new hands
  float minuteAngle = (minutes * 6 - 90) * PI / 180.0;
  float hourAngle = ((hours % 12) * 30 + minutes * 0.5 - 90) * PI / 180.0;
  
  int minuteX = centerX + (radius - 12) * cos(minuteAngle);
  int minuteY = centerY + (radius - 12) * sin(minuteAngle);
  int hourX = centerX + (radius - 30) * cos(hourAngle);
  int hourY = centerY + (radius - 30) * sin(hourAngle);
  
  tft->drawLine(centerX, centerY, hourX, hourY, COLOR_CLOCK_HANDS);
  tft->drawLine(centerX + 1, centerY, hourX + 1, hourY, COLOR_CLOCK_HANDS);
  tft->drawLine(centerX, centerY + 1, hourX, hourY + 1, COLOR_CLOCK_HANDS);
  
  tft->drawLine(centerX, centerY, minuteX, minuteY, COLOR_CLOCK_HANDS);
  tft->drawLine(centerX + 1, centerY, minuteX + 1, minuteY, COLOR_CLOCK_HANDS);
  
  tft->fillCircle(centerX, centerY, 4, COLOR_ACCENT);
}

void DisplayILI9341::drawLargeDigitalTime(int hours, int minutes) {
  int y = 230;
  
  tft->fillRect(0, y - 10, SCREEN_WIDTH, 70, COLOR_BG);
  
  tft->setTextColor(COLOR_TEXT, COLOR_BG);
  tft->setTextSize(1);
  tft->setTextFont(7);
  tft->setTextDatum(MC_DATUM);
  
  char timeStr[6];
  sprintf(timeStr, "%02d:%02d", hours, minutes);
  tft->drawString(timeStr, SCREEN_WIDTH / 2, y + 20);
}

void DisplayILI9341::showInternetRadio(const char* stationName, const char* songTitle, const char* artist) {
  if (currentMode != MODE_INTERNET_RADIO) {
    setMode(MODE_INTERNET_RADIO);
  }
  
  String newInfo1 = String(stationName ? stationName : "");
  String newInfo2 = String(songTitle ? songTitle : "");
  String newInfo3 = String(artist ? artist : "");
  
  if (newInfo1 == lastInfo1 && newInfo2 == lastInfo2 && newInfo3 == lastInfo3) {
    return;
  }
  
  lastInfo1 = newInfo1;
  lastInfo2 = newInfo2;
  lastInfo3 = newInfo3;
  
  clearInfoArea();
  
  tft->setTextColor(COLOR_TEXT, COLOR_BG);
  tft->setTextDatum(MC_DATUM);
  
  int centerX = SCREEN_WIDTH / 2;
  int startY = 60;
  
  if (stationName && strlen(stationName) > 0) {
    tft->setTextFont(4);
    String wrapped = wrapText(stationName, SCREEN_WIDTH - 20, 4);
    tft->drawString(wrapped, centerX, startY);
    startY += 40;
  }
  
  tft->drawLine(40, startY, SCREEN_WIDTH - 40, startY, COLOR_ACCENT);
  startY += 30;
  
  if (songTitle && strlen(songTitle) > 0) {
    tft->setTextFont(2);
    String wrapped = wrapText(songTitle, SCREEN_WIDTH - 30, 2);
    tft->drawString(wrapped, centerX, startY);
    startY += 25;
  }
  
  if (artist && strlen(artist) > 0) {
    tft->setTextColor(COLOR_ACCENT, COLOR_BG);
    tft->setTextFont(2);
    String wrapped = wrapText(artist, SCREEN_WIDTH - 30, 2);
    tft->drawString(wrapped, centerX, startY);
  }
}

void DisplayILI9341::showFMRadio(const char* frequency, const char* rdsStation, const char* rdsText) {
  if (currentMode != MODE_FM_RADIO) {
    setMode(MODE_FM_RADIO);
  }
  
  String newInfo1 = String(frequency ? frequency : "");
  String newInfo2 = String(rdsStation ? rdsStation : "");
  String newInfo3 = String(rdsText ? rdsText : "");
  
  if (newInfo1 == lastInfo1 && newInfo2 == lastInfo2 && newInfo3 == lastInfo3) {
    return;
  }
  
  lastInfo1 = newInfo1;
  lastInfo2 = newInfo2;
  lastInfo3 = newInfo3;
  
  clearInfoArea();
  
  tft->setTextColor(COLOR_TEXT, COLOR_BG);
  tft->setTextDatum(MC_DATUM);
  
  int centerX = SCREEN_WIDTH / 2;
  int startY = 80;
  
  if (frequency && strlen(frequency) > 0) {
    tft->setTextFont(7);
    tft->drawString(frequency, centerX, startY);
    startY += 70;
  }
  
  if (rdsStation && strlen(rdsStation) > 0) {
    tft->setTextFont(4);
    tft->drawString(rdsStation, centerX, startY);
    startY += 40;
  }
  
  if (rdsText && strlen(rdsText) > 0) {
    tft->setTextColor(COLOR_ACCENT, COLOR_BG);
    tft->setTextFont(2);
    String wrapped = wrapText(rdsText, SCREEN_WIDTH - 30, 2);
    tft->drawString(wrapped, centerX, startY);
  }
}

void DisplayILI9341::clearInfoArea() {
  tft->fillRect(10, 30, SCREEN_WIDTH - 20, SCREEN_HEIGHT - 60, COLOR_BG);
}

String DisplayILI9341::wrapText(const char* text, int maxWidth, int textSize) {
  String str = String(text);
  
  tft->setTextFont(textSize);
  if (tft->textWidth(str.c_str()) <= maxWidth) {
    return str;
  }
  
  while (tft->textWidth((str + "...").c_str()) > maxWidth && str.length() > 0) {
    str = str.substring(0, str.length() - 1);
  }
  
  return str + "...";
}

void DisplayILI9341::drawInfo(int volume, int brightness) {
  tft->fillRect(SCREEN_WIDTH - 80, 5, 75, 20, COLOR_BG);
  
  tft->setTextColor(COLOR_ACCENT, COLOR_BG);
  tft->setTextFont(1);
  tft->setTextDatum(TR_DATUM);
  
  char info[20];
  sprintf(info, "V:%d B:%d", volume, brightness);
  tft->drawString(info, SCREEN_WIDTH - 5, 5);
}

void DisplayILI9341::showStartupMessage(const char* message) {
  clear();
  tft->setTextColor(COLOR_TEXT, COLOR_BG);
  tft->setTextFont(4);
  tft->setTextDatum(MC_DATUM);
  tft->drawString(message, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
}

/*
 * TFT_eSPI User_Setup.h Configuration for ILI9341:
 * 
 * #define ILI9341_DRIVER
 * #define TFT_WIDTH  240
 * #define TFT_HEIGHT 320
 * 
 * #define TFT_MISO 19
 * #define TFT_MOSI 23
 * #define TFT_SCLK 18
 * #define TFT_CS   15
 * #define TFT_DC   2
 * #define TFT_RST  4
 * 
 * #define LOAD_GLCD
 * #define LOAD_FONT2
 * #define LOAD_FONT4
 * #define LOAD_FONT6
 * #define LOAD_FONT7
 * #define SMOOTH_FONT
 * 
 * #define SPI_FREQUENCY  40000000
 */