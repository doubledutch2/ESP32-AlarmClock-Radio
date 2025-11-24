/*
 * ESP32 Alarm Clock Radio
 * Simplified working version
 */

#include "Config.h"
#include "DisplayILI9341.h"
#include "TimeModule.h"
#include "FMRadioModule.h"
#include "BuzzerModule.h"
#include "StorageModule.h"

// Module instances
DisplayILI9341* display = nullptr;
TimeModule* timeModule = nullptr;
FMRadioModule* fmRadio = nullptr;
BuzzerModule* buzzer = nullptr;
StorageModule* storage = nullptr;

// ===== ALARM STATE =====
struct AlarmState {
    uint8_t hour;
    uint8_t minute;
    bool enabled;
    bool triggered;
    bool snoozed;
    unsigned long snoozeTime;
} alarmState;

// ===== UI STATE =====
enum MenuState {
    MENU_MAIN,
    MENU_SET_TIME,
    MENU_SET_ALARM,
    MENU_FM_RADIO,
    MENU_SETTINGS
};

struct UIState {
    MenuState currentMenu;
    int selectedItem;
    bool needsRedraw;
    unsigned long lastButtonPress;
} ui;

// Constants
const unsigned long SNOOZE_DURATION = 5 * 60 * 1000;
const unsigned long DEBOUNCE_DELAY = 200;
const unsigned long DISPLAY_UPDATE_INTERVAL = 1000;

// Forward declarations
void handleButtons();
void updateDisplay();
void drawMainScreen();
void drawSetTimeScreen();
void drawSetAlarmScreen();
void drawFMRadioScreen();
void drawSettingsScreen();
void handleMainMenu(bool up, bool down, bool select);
void handleSetTimeMenu(bool up, bool down, bool select);
void handleSetAlarmMenu(bool up, bool down, bool select);
void handleFMRadioMenu(bool up, bool down, bool select);
void handleSettingsMenu(bool up, bool down, bool select);
void checkAlarm();
void triggerAlarm();
void snoozeAlarm();
void saveConfig();

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n====================================");
  Serial.println("   ESP32 Alarm Clock Radio");
  Serial.println("====================================\n");
  
  // Initialize button pins
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_SELECT, INPUT_PULLUP);
  pinMode(BTN_SNOOZE, INPUT_PULLUP);
  
  // Initialize Display
  Serial.println("Initializing Display...");
  display = new DisplayILI9341(TFT_CS, TFT_DC, TFT_RST, TFT_MOSI, TFT_SCLK, TFT_MISO, TFT_BL);
  display->begin();
  display->setBrightness(200);
  display->clear();
  display->drawText(10, 10, "Alarm Clock Starting...", ILI9341_WHITE, 2);
  
  // Initialize Storage
  Serial.println("Initializing Storage...");
  storage = new StorageModule();
  if (storage->begin()) {
    display->drawText(10, 40, "Storage: OK", ILI9341_GREEN, 2);
  } else {
    display->drawText(10, 40, "Storage: FAIL", ILI9341_RED, 2);
  }
  
  // Initialize WiFi and Time
  Serial.println("Initializing WiFi and Time...");
  display->drawText(10, 60, "Connecting WiFi...", ILI9341_YELLOW, 2);
  
  timeModule = new TimeModule("pool.ntp.org", GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC);
  if (timeModule->begin(WIFI_SSID, WIFI_PASSWORD)) {
    Serial.println("WiFi and Time initialized");
    display->drawText(10, 60, "WiFi: OK", ILI9341_GREEN, 2);
    display->drawText(10, 80, timeModule->getIPAddress().c_str(), ILI9341_CYAN, 1);
  } else {
    Serial.println("WiFi/Time initialization failed!");
    display->drawText(10, 60, "WiFi: FAIL", ILI9341_RED, 2);
  }
  
  // Initialize FM Radio
  Serial.println("Initializing FM Radio...");
  Wire.begin(I2C_SDA, I2C_SCL);
  delay(100);
  fmRadio = new FMRadioModule();
  if (fmRadio->begin()) {
    Serial.println("FM Radio initialized");
    display->drawText(10, 100, "FM Radio: OK", ILI9341_GREEN, 2);
  } else {
    Serial.println("FM Radio initialization failed!");
    display->drawText(10, 100, "FM Radio: FAIL", ILI9341_RED, 2);
  }
  
  // Initialize Buzzer
  Serial.println("Initializing Buzzer...");
  buzzer = new BuzzerModule(BUZZER_PIN);
  buzzer->begin();
  buzzer->playBeep();
  display->drawText(10, 120, "Buzzer: OK", ILI9341_GREEN, 2);
  
  // Load alarm configuration
  if (storage && storage->isReady()) {
    uint8_t savedHour, savedMin;
    bool savedEnabled;
    float savedFreq;
    storage->loadConfig(savedHour, savedMin, savedEnabled, savedFreq);
    alarmState.hour = savedHour;
    alarmState.minute = savedMin;
    alarmState.enabled = savedEnabled;
    if (fmRadio) {
      fmRadio->setFrequency(savedFreq);
    }
    display->drawText(10, 140, "Config: Loaded", ILI9341_GREEN, 2);
  } else {
    // Use defaults
    alarmState.hour = 7;
    alarmState.minute = 0;
    alarmState.enabled = false;
  }
  
  alarmState.triggered = false;
  alarmState.snoozed = false;
  alarmState.snoozeTime = 0;
  
  // Initialize UI state
  ui.currentMenu = MENU_MAIN;
  ui.selectedItem = 0;
  ui.needsRedraw = true;
  ui.lastButtonPress = 0;
  
  delay(2000);
  display->clear();
  display->drawClockFace();
  
  Serial.println("Setup complete!\n");
}

void loop() {
  static unsigned long lastUpdate = 0;
  static unsigned long lastWiFiCheck = 0;
  unsigned long now = millis();
  
  // Check WiFi connection periodically
  if (now - lastWiFiCheck >= 30000) {
    if (timeModule && !timeModule->isWiFiConnected()) {
      Serial.println("WiFi disconnected, reconnecting...");
      timeModule->reconnectWiFi();
    }
    lastWiFiCheck = now;
  }
  
  // Handle button input
  handleButtons();
  
  // Update display periodically
  if (now - lastUpdate >= DISPLAY_UPDATE_INTERVAL || ui.needsRedraw) {
    updateDisplay();
    lastUpdate = now;
    ui.needsRedraw = false;
  }
  
  // Check alarm
  checkAlarm();
  
  delay(50);
}

void handleButtons() {
  unsigned long now = millis();
  
  if (now - ui.lastButtonPress < DEBOUNCE_DELAY) {
    return;
  }
  
  bool btnUp = !digitalRead(BTN_UP);
  bool btnDown = !digitalRead(BTN_DOWN);
  bool btnSelect = !digitalRead(BTN_SELECT);
  bool btnSnooze = !digitalRead(BTN_SNOOZE);
  
  if (!btnUp && !btnDown && !btnSelect && !btnSnooze) {
    return;
  }
  
  ui.lastButtonPress = now;
  if (buzzer) buzzer->playBeep();
  
  if (btnSnooze && alarmState.triggered) {
    snoozeAlarm();
    ui.needsRedraw = true;
    return;
  }
  
  switch (ui.currentMenu) {
    case MENU_MAIN:
      handleMainMenu(btnUp, btnDown, btnSelect);
      break;
    case MENU_SET_TIME:
      handleSetTimeMenu(btnUp, btnDown, btnSelect);
      break;
    case MENU_SET_ALARM:
      handleSetAlarmMenu(btnUp, btnDown, btnSelect);
      break;
    case MENU_FM_RADIO:
      handleFMRadioMenu(btnUp, btnDown, btnSelect);
      break;
    case MENU_SETTINGS:
      handleSettingsMenu(btnUp, btnDown, btnSelect);
      break;
  }
}

void handleMainMenu(bool up, bool down, bool select) {
  if (up) {
    ui.selectedItem = (ui.selectedItem - 1 + 4) % 4;
    ui.needsRedraw = true;
  } else if (down) {
    ui.selectedItem = (ui.selectedItem + 1) % 4;
    ui.needsRedraw = true;
  } else if (select) {
    switch (ui.selectedItem) {
      case 0:
        ui.currentMenu = MENU_SET_TIME;
        ui.selectedItem = 0;
        break;
      case 1:
        ui.currentMenu = MENU_SET_ALARM;
        ui.selectedItem = 0;
        break;
      case 2:
        ui.currentMenu = MENU_FM_RADIO;
        ui.selectedItem = 0;
        break;
      case 3:
        alarmState.enabled = !alarmState.enabled;
        saveConfig();
        break;
    }
    if (ui.currentMenu != MENU_MAIN && display) {
      display->resetCache();
    }
    ui.needsRedraw = true;
  }
}

void handleSetTimeMenu(bool up, bool down, bool select) {
  static uint8_t editHour = 12, editMin = 0;
  
  if (ui.selectedItem == 0 && timeModule) {
    editHour = timeModule->getHour();
    editMin = timeModule->getMinute();
  }
  
  if (up) {
    if (ui.selectedItem == 0) {
      editHour = (editHour + 1) % 24;
    } else if (ui.selectedItem == 1) {
      editMin = (editMin + 1) % 60;
    }
    ui.needsRedraw = true;
  } else if (down) {
    if (ui.selectedItem == 0) {
      editHour = (editHour == 0) ? 23 : editHour - 1;
    } else if (ui.selectedItem == 1) {
      editMin = (editMin == 0) ? 59 : editMin - 1;
    }
    ui.needsRedraw = true;
  } else if (select) {
    if (ui.selectedItem < 1) {
      ui.selectedItem++;
    } else {
      if (timeModule) {
        timeModule->setTime(editHour, editMin, 0);
      }
      ui.currentMenu = MENU_MAIN;
      ui.selectedItem = 0;
      if (display) {
        display->clear();
        display->drawClockFace();
        display->resetCache();
      }
    }
    ui.needsRedraw = true;
  }
}

void handleSetAlarmMenu(bool up, bool down, bool select) {
  if (up) {
    if (ui.selectedItem == 0) {
      alarmState.hour = (alarmState.hour + 1) % 24;
    } else if (ui.selectedItem == 1) {
      alarmState.minute = (alarmState.minute + 1) % 60;
    }
    ui.needsRedraw = true;
  } else if (down) {
    if (ui.selectedItem == 0) {
      alarmState.hour = (alarmState.hour == 0) ? 23 : alarmState.hour - 1;
    } else if (ui.selectedItem == 1) {
      alarmState.minute = (alarmState.minute == 0) ? 59 : alarmState.minute - 1;
    }
    ui.needsRedraw = true;
  } else if (select) {
    if (ui.selectedItem < 1) {
      ui.selectedItem++;
    } else {
      saveConfig();
      ui.currentMenu = MENU_MAIN;
      ui.selectedItem = 0;
      if (display) {
        display->clear();
        display->drawClockFace();
        display->resetCache();
      }
    }
    ui.needsRedraw = true;
  }
}

void handleFMRadioMenu(bool up, bool down, bool select) {
  if (!fmRadio) return;
  
  if (up) {
    if (ui.selectedItem == 0) {
      float freq = fmRadio->getFrequency();
      freq += 0.1;
      if (freq > 108.0) freq = 87.0;
      fmRadio->setFrequency(freq);
    } else if (ui.selectedItem == 1) {
      fmRadio->seekUp();
    }
    ui.needsRedraw = true;
  } else if (down) {
    if (ui.selectedItem == 0) {
      float freq = fmRadio->getFrequency();
      freq -= 0.1;
      if (freq < 87.0) freq = 108.0;
      fmRadio->setFrequency(freq);
    } else if (ui.selectedItem == 1) {
      fmRadio->seekDown();
    }
    ui.needsRedraw = true;
  } else if (select) {
    if (ui.selectedItem == 0) {
      ui.selectedItem = 1;
    } else {
      saveConfig();
      ui.currentMenu = MENU_MAIN;
      ui.selectedItem = 0;
      if (display) {
        display->clear();
        display->drawClockFace();
        display->resetCache();
      }
    }
    ui.needsRedraw = true;
  }
}

void handleSettingsMenu(bool up, bool down, bool select) {
  if (select) {
    ui.currentMenu = MENU_MAIN;
    ui.selectedItem = 0;
    if (display) {
      display->clear();
      display->drawClockFace();
      display->resetCache();
    }
    ui.needsRedraw = true;
  }
}

void updateDisplay() {
  if (!display) return;
  
  switch (ui.currentMenu) {
    case MENU_MAIN:
      drawMainScreen();
      break;
    case MENU_SET_TIME:
      display->clear();
      drawSetTimeScreen();
      break;
    case MENU_SET_ALARM:
      display->clear();
      drawSetAlarmScreen();
      break;
    case MENU_FM_RADIO:
      display->clear();
      drawFMRadioScreen();
      break;
    case MENU_SETTINGS:
      display->clear();
      drawSettingsScreen();
      break;
  }
}

void drawMainScreen() {
  if (!display || !timeModule) return;
  
  display->updateTime(timeModule->getHour(), timeModule->getMinute(), timeModule->getSecond());
  display->updateDate(timeModule->getYear(), timeModule->getMonth(), timeModule->getDay());
  display->updateAlarmStatus(alarmState.enabled, alarmState.hour, alarmState.minute);
  
  if (fmRadio) {
    display->updateFMFrequency(fmRadio->getFrequency());
  }
  
  display->updateWiFiStatus(timeModule->isWiFiConnected());
  
  static bool menuDrawn = false;
  if (!menuDrawn) {
    display->drawText(10, 220, "UP/DN:Menu SEL:Choose", ILI9341_CYAN, 1);
    menuDrawn = true;
  }
}

void drawSetTimeScreen() {
  if (!display || !timeModule) return;
  
  display->drawText(80, 20, "SET TIME", ILI9341_YELLOW, 3);
  
  char timeStr[6];
  sprintf(timeStr, "%02d:%02d", timeModule->getHour(), timeModule->getMinute());
  
  uint16_t color = (ui.selectedItem == 0) ? ILI9341_GREEN : ILI9341_WHITE;
  display->drawText(60, 100, timeStr, color, 4);
  
  display->drawText(10, 180, "UP/DN:Change SEL:Save", ILI9341_CYAN, 1);
  display->drawText(10, 200, "Note: Syncs with NTP", ILI9341_YELLOW, 1);
}

void drawSetAlarmScreen() {
  if (!display) return;
  
  display->drawText(70, 20, "SET ALARM", ILI9341_YELLOW, 3);
  
  char alarmStr[6];
  sprintf(alarmStr, "%02d:%02d", alarmState.hour, alarmState.minute);
  
  uint16_t color = (ui.selectedItem == 0) ? ILI9341_GREEN : ILI9341_WHITE;
  display->drawText(60, 100, alarmStr, color, 4);
  
  display->drawText(30, 180, "UP/DN:Change SEL:Save", ILI9341_CYAN, 1);
}

void drawFMRadioScreen() {
  if (!display) return;
  
  display->drawText(80, 20, "FM RADIO", ILI9341_YELLOW, 3);
  
  if (fmRadio) {
    char freqStr[12];
    sprintf(freqStr, "%.1f MHz", fmRadio->getFrequency());
    display->drawText(70, 100, freqStr, ILI9341_WHITE, 3);
  } else {
    display->drawText(50, 100, "Not Available", ILI9341_RED, 2);
  }
  
  display->drawText(20, 180, "UP/DN:Tune/Seek SEL:Back", ILI9341_CYAN, 1);
}

void drawSettingsScreen() {
  if (!display) return;
  
  display->drawText(70, 20, "SETTINGS", ILI9341_YELLOW, 3);
  display->drawText(50, 100, "Press SELECT", ILI9341_WHITE, 2);
  display->drawText(70, 130, "to return", ILI9341_WHITE, 2);
}

void checkAlarm() {
  if (!alarmState.enabled || alarmState.triggered || !timeModule) {
    return;
  }
  
  if (alarmState.snoozed && millis() >= alarmState.snoozeTime) {
    triggerAlarm();
    alarmState.snoozed = false;
    return;
  }
  
  uint8_t currentHour = timeModule->getHour();
  uint8_t currentMin = timeModule->getMinute();
  uint8_t currentSec = timeModule->getSecond();
  
  if (currentHour == alarmState.hour && currentMin == alarmState.minute && currentSec == 0) {
    triggerAlarm();
  }
}

void triggerAlarm() {
  alarmState.triggered = true;
  if (buzzer) buzzer->playAlarm();
  if (display) {
    display->clear();
    display->drawText(60, 100, "WAKE UP!", ILI9341_RED, 4);
    display->drawText(40, 150, "Press SNOOZE", ILI9341_WHITE, 2);
  }
}

void snoozeAlarm() {
  alarmState.triggered = false;
  alarmState.snoozed = true;
  alarmState.snoozeTime = millis() + SNOOZE_DURATION;
  if (buzzer) buzzer->stopTone();
  
  if (display) {
    display->clear();
    display->drawText(70, 100, "SNOOZED", ILI9341_YELLOW, 3);
    display->drawText(60, 140, "5 minutes", ILI9341_WHITE, 2);
  }
  delay(2000);
}

void saveConfig() {
  if (storage && storage->isReady()) {
    float fmFreq = fmRadio ? fmRadio->getFrequency() : 98.0;
    storage->saveConfig(alarmState.hour, alarmState.minute, alarmState.enabled, fmFreq);
    Serial.println("Configuration saved");
  }
}