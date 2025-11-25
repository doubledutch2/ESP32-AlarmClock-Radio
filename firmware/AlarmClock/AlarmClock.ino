/*
 * ESP32 Alarm Clock Radio - Complete Version
 * With Internet Radio, FM Radio, Web Interface, and Alarm Clock
 */

#include "Config.h"
#include "DisplayILI9341.h"
#include "TimeModule.h"
#include "FMRadioModule.h"
#include "BuzzerModule.h"
#include "StorageModule.h"
#include "WiFiModule.h"
#include "AudioModule.h"
#include "WebServerModule.h"
#include "LEDModule.h"

// Module instances
DisplayILI9341* display = nullptr;
TimeModule* timeModule = nullptr;
FMRadioModule* fmRadio = nullptr;
BuzzerModule* buzzer = nullptr;
StorageModule* storage = nullptr;
WiFiModule* wifi = nullptr;
AudioModule* audio = nullptr;
WebServerModule* webServer = nullptr;
LEDModule* led = nullptr;

// Default stations
RadioStation defaultStations[] = {
  {"BBC 5 Live", "https://stream.live.vc.bbcmedia.co.uk/bbc_radio_five_live"},
  {"Veronica", "https://playerservices.streamtheworld.com/api/livestream-redirect/VERONICAAAC.aac"},
  {"NPO Radio 1", "https://icecast.omroep.nl/radio1-bb-mp3"},
  {"ERT Kosmos", "https://radiostreaming.ert.gr/ert-kosmos"},
  {"Real FM", "https://realfm.live24.gr/realfm"}
};

RadioStation* stationList = nullptr;
int stationCount = 0;

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
    MENU_STATIONS,
    MENU_SETTINGS
};

struct UIState {
    MenuState currentMenu;
    int selectedItem;
    bool needsRedraw;
    unsigned long lastButtonPress;
} ui;

// Button state
uint8_t brightnessLevel = 3;
int lastVolumePotValue = -1;

// Constants
const unsigned long SNOOZE_DURATION = 5 * 60 * 1000;
const unsigned long DEBOUNCE_DELAY = 200;
const unsigned long DISPLAY_UPDATE_INTERVAL = 1000;

// Forward declarations
void handleButtons();
void checkVolumeControl();
void updateDisplay();
void drawMainScreen();
void drawSetTimeScreen();
void drawSetAlarmScreen();
void drawFMRadioScreen();
void drawStationsScreen();
void drawSettingsScreen();
void handleMainMenu(bool up, bool down, bool select);
void handleSetTimeMenu(bool up, bool down, bool select);
void handleSetAlarmMenu(bool up, bool down, bool select);
void handleFMRadioMenu(bool up, bool down, bool select);
void handleStationsMenu(bool up, bool down, bool select);
void handleSettingsMenu(bool up, bool down, bool select);
void checkAlarm();
void triggerAlarm();
void snoozeAlarm();
void saveConfig();
void loadStationsFromStorage();
void playCustomStation(const char* name, const char* url);

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n====================================");
  Serial.println("   ESP32 Alarm Clock Radio");
  Serial.println("====================================\n");
  
  // Initialize LED first
  if (ENABLE_LED) {
    led = new LEDModule(LED_PIN);
    led->begin();
    led->setColor(LEDModule::COLOR_RED, BRIGHT_DIM);
  }
  
  // Initialize button pins
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_SELECT, INPUT_PULLUP);
  pinMode(BTN_SNOOZE, INPUT_PULLUP);
  pinMode(BRIGHTNESS_PIN, INPUT);
  pinMode(NEXT_STATION_PIN, INPUT);
  pinMode(VOL_PIN, INPUT);
  
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
    if (led) led->setColor(LEDModule::COLOR_YELLOW, BRIGHT_DIM);
  } else {
    display->drawText(10, 40, "Storage: FAIL", ILI9341_RED, 2);
  }
  
  // Initialize WiFi
  Serial.println("Initializing WiFi...");
  display->drawText(10, 60, "Connecting WiFi...", ILI9341_YELLOW, 2);
  
  wifi = new WiFiModule(WIFI_SSID, WIFI_PASSWORD);
  if (wifi->connect()) {
    display->drawText(10, 60, "WiFi: OK", ILI9341_GREEN, 2);
    display->drawText(10, 80, wifi->getLocalIP().c_str(), ILI9341_CYAN, 1);
    if (led) led->setColor(LEDModule::COLOR_GREEN, BRIGHT_DIM);
  } else {
    display->drawText(10, 60, "WiFi: FAIL", ILI9341_RED, 2);
    if (led) led->setColor(LEDModule::COLOR_RED, BRIGHT_FULL);
  }
  
  // Initialize Time Module
  Serial.println("Initializing Time...");
  timeModule = new TimeModule("pool.ntp.org", GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC);
  if (timeModule->begin(WIFI_SSID, WIFI_PASSWORD)) {
    display->drawText(10, 100, "Time: OK", ILI9341_GREEN, 2);
  } else {
    display->drawText(10, 100, "Time: FAIL", ILI9341_RED, 2);
  }
  
  // Initialize Web Server
  if (ENABLE_WEB && wifi->isConnected()) {
    webServer = new WebServerModule();
    webServer->setPlayCallback(playCustomStation);
    webServer->begin(MDNS_NAME);
    display->drawText(10, 120, "Web: OK", ILI9341_GREEN, 2);
    Serial.printf("Web interface: http://%s.local or http://%s\n", MDNS_NAME, wifi->getLocalIP().c_str());
  }
  
  // Load stations
  loadStationsFromStorage();
  
  // Initialize Audio
  if (ENABLE_AUDIO) {
    Serial.println("Initializing Audio...");
    audio = new AudioModule(I2S_BCLK, I2S_LRC, I2S_DOUT_L, MAX_VOLUME);
    audio->begin();
    audio->setStationList(stationList, stationCount);
    audio->setVolume(10);
    display->drawText(10, 140, "Audio: OK", ILI9341_GREEN, 2);
  }
  
  // Initialize FM Radio
  if (ENABLE_FM_RADIO) {
    Serial.println("Initializing FM Radio...");
    Wire.begin(I2C_SDA, I2C_SCL);
    delay(100);
    fmRadio = new FMRadioModule();
    if (fmRadio->begin()) {
      Serial.println("FM Radio initialized");
      display->drawText(10, 160, "FM Radio: OK", ILI9341_GREEN, 2);
    } else {
      Serial.println("FM Radio initialization failed!");
      display->drawText(10, 160, "FM Radio: FAIL", ILI9341_RED, 2);
    }
  }
  
  // Initialize Buzzer
  Serial.println("Initializing Buzzer...");
  buzzer = new BuzzerModule(BUZZER_PIN);
  buzzer->begin();
  buzzer->playBeep();
  display->drawText(10, 180, "Buzzer: OK", ILI9341_GREEN, 2);
  
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
  } else {
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
  
  if (led) led->setColor(LEDModule::COLOR_BLUE, BRIGHT_DIM);
  Serial.println("Setup complete!\n");
}

void loop() {
  static unsigned long lastUpdate = 0;
  unsigned long now = millis();
  
  // Check WiFi
  if (wifi) wifi->checkConnection();
  
  // Handle web requests
  if (webServer) webServer->handleClient();
  
  // Audio loop
  if (audio) audio->loop();
  
  // Handle buttons
  handleButtons();
  
  // Handle volume control
  checkVolumeControl();
  
  // Update display periodically
  if (now - lastUpdate >= DISPLAY_UPDATE_INTERVAL || ui.needsRedraw) {
    updateDisplay();
    lastUpdate = now;
    ui.needsRedraw = false;
  }
  
  // Check alarm
  checkAlarm();
  
  delay(1);
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
  
  // Brightness button
  static bool brightnessPressed = false;
  int brightnessButton = digitalRead(BRIGHTNESS_PIN);
  if (brightnessButton == HIGH && !brightnessPressed) {
    brightnessPressed = true;
    brightnessLevel = (brightnessLevel + 1) % 6;
    if (display) display->setBrightness(brightnessLevel * 50);
  } else if (brightnessButton == LOW) {
    brightnessPressed = false;
  }
  
  // Next station button
  static bool nextStationPressed = false;
  int nextButton = digitalRead(NEXT_STATION_PIN);
  if (nextButton == HIGH && !nextStationPressed) {
    nextStationPressed = true;
    if (audio) audio->nextStation();
  } else if (nextButton == LOW) {
    nextStationPressed = false;
  }
  
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
    case MENU_STATIONS:
      handleStationsMenu(btnUp, btnDown, btnSelect);
      break;
    case MENU_SETTINGS:
      handleSettingsMenu(btnUp, btnDown, btnSelect);
      break;
  }
}

void checkVolumeControl() {
  if (!audio) return;
  
  int potValue = analogRead(VOL_PIN);
  
  if (lastVolumePotValue == -1 || abs(potValue - lastVolumePotValue) > 40) {
    lastVolumePotValue = potValue;
    int newVolume = map(potValue, 0, 4095, 0, audio->getMaxVolume());
    audio->setVolume(newVolume);
  }
}

void handleMainMenu(bool up, bool down, bool select) {
  if (up) {
    ui.selectedItem = (ui.selectedItem - 1 + 5) % 5;
    ui.needsRedraw = true;
  } else if (down) {
    ui.selectedItem = (ui.selectedItem + 1) % 5;
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
        ui.currentMenu = MENU_STATIONS;
        ui.selectedItem = 0;
        break;
      case 4:
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
      if (timeModule) timeModule->setTime(editHour, editMin, 0);
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
    float freq = fmRadio->getFrequency();
    freq += 0.1;
    if (freq > 108.0) freq = 87.0;
    fmRadio->setFrequency(freq);
    ui.needsRedraw = true;
  } else if (down) {
    float freq = fmRadio->getFrequency();
    freq -= 0.1;
    if (freq < 87.0) freq = 108.0;
    fmRadio->setFrequency(freq);
    ui.needsRedraw = true;
  } else if (select) {
    saveConfig();
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

void handleStationsMenu(bool up, bool down, bool select) {
  if (up) {
    ui.selectedItem = (ui.selectedItem - 1 + stationCount) % stationCount;
    ui.needsRedraw = true;
  } else if (down) {
    ui.selectedItem = (ui.selectedItem + 1) % stationCount;
    ui.needsRedraw = true;
  } else if (select) {
    if (audio && ui.selectedItem < stationCount) {
      audio->playStation(ui.selectedItem);
    }
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
    case MENU_STATIONS:
      display->clear();
      drawStationsScreen();
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
  
  display->updateWiFiStatus(wifi && wifi->isConnected());
  
  // Show playing station
  if (audio && audio->getIsPlaying()) {
    static String lastStation = "";
    String currentStation = audio->getCurrentStationName();
    if (currentStation != lastStation) {
      display->fillRect(10, 195, 200, 20, ILI9341_BLACK);
      display->drawText(10, 195, currentStation.c_str(), ILI9341_YELLOW, 1);
      lastStation = currentStation;
    }
  }
  
  // Show volume
  if (audio) {
    static int lastVol = -1;
    int vol = audio->getCurrentVolume();
    if (vol != lastVol) {
      display->fillRect(220, 220, 90, 15, ILI9341_BLACK);
      char volStr[20];
      sprintf(volStr, "Vol:%d", vol);
      display->drawText(220, 220, volStr, ILI9341_CYAN, 1);
      lastVol = vol;
    }
  }
  
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
  }
  
  display->drawText(20, 180, "UP/DN:Tune SEL:Back", ILI9341_CYAN, 1);
}

void drawStationsScreen() {
  if (!display) return;
  
  display->drawText(60, 20, "STATIONS", ILI9341_YELLOW, 3);
  
  int startIdx = (ui.selectedItem / 5) * 5;
  for (int i = 0; i < 5 && (startIdx + i) < stationCount; i++) {
    uint16_t color = (startIdx + i == ui.selectedItem) ? ILI9341_GREEN : ILI9341_WHITE;
    display->drawText(10, 60 + i * 25, stationList[startIdx + i].name.c_str(), color, 2);
  }
  
  display->drawText(10, 200, "UP/DN:Select SEL:Play", ILI9341_CYAN, 1);
}

void drawSettingsScreen() {
  if (!display) return;
  
  display->drawText(70, 20, "SETTINGS", ILI9341_YELLOW, 3);
  display->drawText(30, 80, "Web Interface:", ILI9341_WHITE, 2);
  if (wifi && wifi->isConnected()) {
    display->drawText(30, 110, wifi->getLocalIP().c_str(), ILI9341_GREEN, 2);
  }
  display->drawText(50, 150, "Press SELECT", ILI9341_WHITE, 2);
  display->drawText(70, 170, "to return", ILI9341_WHITE, 2);
}

void checkAlarm() {
  if (!alarmState.enabled || alarmState.triggered || !timeModule) return;
  
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
  if (audio && stationCount > 0) audio->playStation(0);
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

void loadStationsFromStorage() {
  if (!storage) {
    stationCount = sizeof(defaultStations) / sizeof(RadioStation);
    stationList = defaultStations;
    Serial.printf("Using %d default stations\n", stationCount);
    return;
  }
  
  stationCount = storage->getStationCount();
  
  if (stationCount > 0) {
    stationList = new RadioStation[stationCount];
    for (int i = 0; i < stationCount; i++) {
      RadioStation* station = storage->getStation(i);
      if (station) {
        stationList[i].name = String(station->name);
        stationList[i].frequency = station->frequency;
      }
    }
    Serial.printf("Loaded %d stations from storage\n", stationCount);
  } else {
    stationCount = sizeof(defaultStations) / sizeof(RadioStation);
    stationList = defaultStations;
    Serial.printf("Using %d default stations\n", stationCount);
  }
}

void playCustomStation(const char* name, const char* url) {
  Serial.printf("Web request: Play %s - %s\n", name, url);
  if (audio) {
    audio->playCustom(name, url);
  }
}