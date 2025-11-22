// ==================== DisplayILI9341.h ====================
#ifndef DISPLAY_ILI9341_H
#define DISPLAY_ILI9341_H

#include "DisplayInterface.h"
#include <TFT_eSPI.h>

enum DisplayMode {
  MODE_CLOCK = 0,
  MODE_INTERNET_RADIO,
  MODE_FM_RADIO
};

class DisplayILI9341 : public DisplayInterface {
private:
  TFT_eSPI* tft;
  uint8_t brightnessLevel;
  uint8_t backlightPin;
  DisplayMode currentMode;
  
  static const int SCREEN_WIDTH = 240;
  static const int SCREEN_HEIGHT = 320;
  
  // Colors (RGB565) - Day/Night modes
  uint16_t COLOR_BG;
  uint16_t COLOR_TEXT;
  uint16_t COLOR_CLOCK_FACE;
  uint16_t COLOR_CLOCK_HANDS;
  uint16_t COLOR_ACCENT;
  
  static const uint16_t DAY_BG = 0xFFFF;
  static const uint16_t DAY_TEXT = 0x0000;
  static const uint16_t DAY_ACCENT = 0x001F;
  
  static const uint16_t NIGHT_BG = 0x0000;
  static const uint16_t NIGHT_TEXT = 0x8410;
  static const uint16_t NIGHT_ACCENT = 0x8000;
  
  int lastHour;
  int lastMinute;
  String lastInfo1;
  String lastInfo2;
  String lastInfo3;
  
  void drawAnalogClock(int hours, int minutes, bool forceRedraw = false);
  void drawLargeDigitalTime(int hours, int minutes);
  void drawStationInfo(const char* line1, const char* line2 = nullptr, const char* line3 = nullptr);
  void clearInfoArea();
  void setNightMode(bool night);
  String wrapText(const char* text, int maxWidth, int textSize);
  
public:
  DisplayILI9341(uint8_t backlightPin = 0);
  ~DisplayILI9341();
  
  bool begin() override;
  void clear() override;
  void update() override;
  void setBrightness(uint8_t level) override;
  
  void setMode(DisplayMode mode);
  DisplayMode getMode() { return currentMode; }
  
  void drawClock(int hours, int minutes, int seconds) override;
  void showInternetRadio(const char* stationName, const char* songTitle = nullptr, const char* artist = nullptr);
  void showFMRadio(const char* frequency, const char* rdsStation = nullptr, const char* rdsText = nullptr);
  
  void drawInfo(int volume, int brightness) override;
  void showStartupMessage(const char* message) override;
  void enableNightMode(bool enable);
};

#endif
