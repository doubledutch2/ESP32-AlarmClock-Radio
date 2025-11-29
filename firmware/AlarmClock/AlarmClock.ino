/*
 * ESP32 Alarm Clock Radio - Modular Main File
 * Simplified main file - business logic is in separate modules
 */

#include "Config.h"
#include "CommonTypes.h"
#include "HardwareSetup.h"
#include "MenuSystem.h"
#include "AlarmController.h"

// Module instances (managed by HardwareSetup)
HardwareSetup* hardware = nullptr;
MenuSystem* menu = nullptr;
AlarmController* alarmController = nullptr;

// State
AlarmState alarmState;
UIState uiState;

// Internet radio stations
InternetRadioStation defaultStations[] = {
  {"BBC 5 Live", "https://stream.live.vc.bbcmedia.co.uk/bbc_radio_five_live"},
  {"Veronica", "https://playerservices.streamtheworld.com/api/livestream-redirect/VERONICAAAC.aac"},
  {"NPO Radio 1", "https://icecast.omroep.nl/radio1-bb-mp3"}
};
InternetRadioStation* stationList = defaultStations;
int stationCount = 3;

// Constants
const unsigned long DEBOUNCE_DELAY = 200;
const unsigned long DISPLAY_UPDATE_INTERVAL = 1000;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n====================================");
  Serial.println("   ESP32 Alarm Clock Radio");
  Serial.println("====================================\n");
  
  Serial.println("Going to initialize PSRAM");
  if (psramInit()) {
    Serial.print("PSRAM initialized. Total PSRAM size: ");
    Serial.println(ESP.getPsramSize()); // Total size in bytes
    Serial.print("Free PSRAM: ");
    Serial.println(ESP.getFreePsram()); // Free memory in bytes
  } else {
    Serial.println("PSRAM not found or not initialized.");
  }
  
  // Initialize all hardware
  Serial.println("Before hardware setup");
  hardware = new HardwareSetup();
  if (!hardware->begin()) {
    Serial.println("Hardware initialization failed!");
    while(1) delay(1000);
  }

  Serial.println("Going to initialize PSRAM");
  if (psramInit()) {
    Serial.print("PSRAM initialized. Total PSRAM size: ");
    Serial.println(ESP.getPsramSize()); // Total size in bytes
    Serial.print("Free PSRAM: ");
    Serial.println(ESP.getFreePsram()); // Free memory in bytes
  } else {
    Serial.println("PSRAM not found or not initialized.");
  }
  
  // Initialize menu system
  menu = new MenuSystem(
    hardware->getDisplay(),
    hardware->getTimeModule(),
    hardware->getFMRadio(),
    hardware->getAudio(),
    hardware->getBuzzer(),
    hardware->getStorage()
  );
  
  // Initialize alarm controller
  alarmController = new AlarmController(
    hardware->getBuzzer(),
    hardware->getAudio(),
    hardware->getDisplay()
  );
  
  // Setup state
  alarmState.hour = 7;
  alarmState.minute = 0;
  alarmState.enabled = false;
  alarmState.triggered = false;
  alarmState.snoozed = false;
  
  uiState.currentMenu = MENU_MAIN;
  uiState.selectedItem = 0;
  uiState.needsRedraw = true;
  uiState.lastButtonPress = 0;
  
  // Load configuration
  if (hardware->getStorage() && hardware->getStorage()->isReady()) {
    uint8_t h, m;
    bool en;
    float freq;
    hardware->getStorage()->loadConfig(h, m, en, freq);
    alarmState.hour = h;
    alarmState.minute = m;
    alarmState.enabled = en;
    if (hardware->getFMRadio()) {
      hardware->getFMRadio()->setFrequency(freq);
    }
  }
  
  // Connect states to menu system
  menu->setAlarmState(&alarmState);
  menu->setUIState(&uiState);
  menu->setStationList(stationList, stationCount);
  
  // Setup web callback
  if (hardware->getWebServer()) {
    hardware->getWebServer()->setPlayCallback([](const char* name, const char* url) {
      if (hardware->getAudio()) {
        hardware->getAudio()->playCustom(name, url);
      }
    });
  }
  
  Serial.println("Setup complete!\n");
}

void loop() {
  static unsigned long lastUpdate = 0;
  unsigned long now = millis();
  
  // Update all hardware
  hardware->loop();
  
  // Read buttons
  bool btnUp = !digitalRead(BTN_UP);
  bool btnDown = !digitalRead(BTN_DOWN);
  bool btnSelect = !digitalRead(BTN_SELECT);
  bool btnSnooze = !digitalRead(BTN_SNOOZE);
  
  // Handle buttons (with debouncing in menu system)
  if (now - uiState.lastButtonPress >= DEBOUNCE_DELAY) {
    if (btnUp || btnDown || btnSelect || btnSnooze) {
      menu->handleButtons(btnUp, btnDown, btnSelect, btnSnooze);
      uiState.lastButtonPress = now;
    }
  }
  
  // Update display
  if (now - lastUpdate >= DISPLAY_UPDATE_INTERVAL || uiState.needsRedraw) {
    menu->updateDisplay();
    menu->setWiFiStatus(hardware->getWiFi() && hardware->getWiFi()->isConnected());
    lastUpdate = now;
    uiState.needsRedraw = false;
  }
  
  // Check alarm
  alarmController->checkAlarm(&alarmState, hardware->getTimeModule());
  
  delay(1);
}