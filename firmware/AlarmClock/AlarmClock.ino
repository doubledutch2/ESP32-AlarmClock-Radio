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

// Internet radio stations - dynamically loaded from storage
InternetRadioStation* stationList = nullptr;
int stationCount = 0;

// Constants
const unsigned long DEBOUNCE_DELAY = 200;
const unsigned long DISPLAY_UPDATE_INTERVAL = 1000;

void loadStationsFromStorage() {
  if (!hardware || !hardware->getStorage()) return;
  
  StorageModule* storage = hardware->getStorage();
  int savedCount = storage->getInternetStationCount();
  
  Serial.printf("Loading %d stations from storage\n", savedCount);
  
  // Free old station list if it exists
  if (stationList != nullptr) {
    delete[] stationList;
    stationList = nullptr;
  }
  
  // If no stations saved, create default stations
  if (savedCount == 0) {
    Serial.println("No stations in storage, using defaults");
    stationCount = 5;
    stationList = new InternetRadioStation[stationCount];
    
    stationList[0].name = "BBC World Service";
    stationList[0].url = "http://stream.live.vc.bbcmedia.co.uk/bbc_world_service";

    stationList[1].name = "Radio Paradise";
    stationList[1].url = "http://stream.radioparadise.com/aac-128";

    stationList[2].name = "LBC";
    stationList[2].url = "https://ice-sov.musicradio.com/LBC973";
    
    stationList[3].name = "Veronica";
    stationList[3].url = "https://playerservices.streamtheworld.com/api/livestream-redirect/VERONICAAAC.aac";
    
    stationList[4].name = "NPO Radio 1";
    stationList[4].url = "https://icecast.omroep.nl/radio1-bb-mp3";

    

    
    // Save defaults to storage
    for (int i = 0; i < stationCount; i++) {
      storage->saveInternetStation(i, stationList[i].name.c_str(), stationList[i].url.c_str());
    }
    storage->setInternetStationCount(stationCount);
  } else {
    // Load stations from storage
    stationCount = savedCount;
    stationList = new InternetRadioStation[stationCount];
    
    for (int i = 0; i < stationCount; i++) {
      String name, url;
      if (storage->loadInternetStation(i, name, url)) {
        stationList[i].name = name;
        stationList[i].url = url;
        Serial.printf("Loaded station %d: %s\n", i, name.c_str());
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n====================================");
  Serial.println("   ESP32 Alarm Clock Radio");
  Serial.println("====================================\n");
  
  Serial.println("Going to initialize PSRAM");
  if (psramInit()) {
    Serial.print("PSRAM initialized. Total PSRAM size: ");
    Serial.println(ESP.getPsramSize());
    Serial.print("Free PSRAM: ");
    Serial.println(ESP.getFreePsram());
  } else {
    Serial.println("PSRAM not found or not initialized.");
  }
  
  Serial.printf("LittleFS used: %d bytes\n", LittleFS.usedBytes());
  Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
  Serial.printf("Total heap: %d bytes\n", ESP.getHeapSize());
  Serial.println();

  // Initialize all hardware
  Serial.println("Before hardware setup");
  hardware = new HardwareSetup();
  if (!hardware->begin()) {
    Serial.println("Hardware initialization failed!");
    while(1) delay(1000);
  }

  // Initialize the MODE_SWITCH_PIN pin so it connectes to the NO (IN=LOW)

  pinMode(MODE_SWITCH_PIN, OUTPUT);
  digitalWrite(MODE_SWITCH_PIN, LOW);

  // Load stations from storage (must be done AFTER hardware init)
  loadStationsFromStorage();

  // Initialize menu system
  menu = new MenuSystem(
    hardware->getDisplay(),
    hardware->getTimeModule(),
    hardware->getFMRadio(),
    hardware->getAudio(),
    hardware->getStorage()
  );
  
  // Initialize alarm controller with all required parameters
  Serial.println("Creating AlarmController...");
  alarmController = new AlarmController(
    hardware->getAudio(),
    hardware->getFMRadio(),
    hardware->getDisplay(),
    hardware->getStorage()
  );
  
  // CRITICAL: Load alarms from storage
  Serial.println("Loading alarms from storage...");
  alarmController->begin();
  Serial.println("Alarms loaded.");
  
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
    
    // Load timezone settings (for future use - requires restart to apply)
    long gmtOffset, dstOffset;
    hardware->getStorage()->loadTimezone(gmtOffset, dstOffset);
    Serial.printf("Timezone loaded: GMT offset %ld seconds, DST offset %d seconds\n", 
                  gmtOffset, dstOffset);
  }
  
  // Connect states to menu system
  menu->setAlarmState(&alarmState);
  menu->setUIState(&uiState);
  menu->setStationList(stationList, stationCount);
  
  // Pass storage and time module to web server
  if (hardware->getWebServer()) {
    Serial.println("Configuring web server with station list...");
    hardware->getWebServer()->setStationList(stationList, stationCount);
    
    // CRITICAL: Pass AlarmController reference so web can reload alarms
    hardware->getWebServer()->setAlarmController(alarmController);
    
    // Setup web callback
    hardware->getWebServer()->setPlayCallback([](const char* name, const char* url) {
      if (hardware->getAudio()) {
        hardware->getAudio()->playCustom(name, url);
      }
    });
    
    Serial.printf("Web server ready with %d stations\n", stationCount);
  }
  
  // Pass station list to audio module
  if (hardware->getAudio()) {
    hardware->getAudio()->setStationList(stationList, stationCount);
  }
  
  Serial.println("Setup complete!\n");
  Serial.printf("Loaded %d internet radio stations\n", stationCount);
  Serial.println("Web interface available at:");
  if (hardware->getWiFi()) {
    Serial.printf("  http://%s\n", hardware->getWiFi()->getLocalIP().c_str());
    Serial.printf("  http://%s.local\n", MDNS_NAME);
    Serial.printf("  http://%s.local/alarms - Alarm Management\n", MDNS_NAME);
  }
  
  Serial.println("\n*** System Ready - Monitoring for alarms ***\n");
}

void loop() {
  static unsigned long lastUpdate = 0;
  unsigned long now = millis();
  // static boolean  pinHigh = false;
  // static int      pinCounter = 0;
  
  yield();

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

  // Play with the MODE_SWITCH_PIN
  /*
  if (pinCounter == 1000) {
    if (pinHigh) {
      digitalWrite(MODE_SWITCH_PIN, LOW);
      Serial.println("Set Mode Switch Pin: Low");
      pinHigh = false;
    }
    else {
      digitalWrite(MODE_SWITCH_PIN,HIGH);
      Serial.println("Set Mode Switch Pin: High");
      pinHigh = true;
    }
    pinCounter = 0;
  }
  pinCounter++;
  */

  // Check alarm
  alarmController->checkAlarms(hardware->getTimeModule());
  delay(1);
}