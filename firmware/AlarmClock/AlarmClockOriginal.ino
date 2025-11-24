#include <esp_partition.h>
#include "config.h"
#include "DisplayInterface.h"
#include "WiFiModule.h"
#include "WebServerModule.h"
#include "AudioModule.h"
#include "TimeModule.h"
#include "LEDModule.h"
#include "StorageModule.h"

#if ENABLE_FM_RADIO
  #include "FMRadioModule.h"
#endif
#include "DisplayInterface.h"
#include "WiFiModule.h"
#include "WebServerModule.h"
#include "AudioModule.h"
#include "TimeModule.h"
#include "LEDModule.h"
#include "StorageModule.h"

// Choose display type based on config
#ifdef USE_OLED_DISPLAY
  #include "DisplayOLED.h"
  DisplayOLED* display;
#elif defined(USE_TFT_DISPLAY)
  #include "DisplayTFT.h"
  DisplayTFT* display;
#else
  DisplayInterface* display = nullptr;
#endif

// Module instances
WiFiModule* wifi = nullptr;
WebServerModule* webServer = nullptr;
AudioModule* audio = nullptr;
TimeModule* timeModule = nullptr;
LEDModule* led = nullptr;
StorageModule* storage = nullptr;

#if ENABLE_FM_RADIO
  FMRadioModule* fmRadio = nullptr;
#endif

// Audio source control
enum AudioSource {
  SOURCE_INTERNET_RADIO = 0,
  SOURCE_FM_RADIO = 1
};

AudioSource currentAudioSource = SOURCE_INTERNET_RADIO;

// Default station list (will be overridden by stored stations if available)
RadioStation defaultStations[] = {
  {"BBC 5 Live", "https://a.files.bbci.co.uk/ms6/live/3441A116-B12E-4D2F-ACA8-C1984642FA4B/audio/simulcast/dash/nonuk/pc_hd_abr_v2/cfs/bbc_radio_five_live.mpd"},
  {"Veronica", "https://playerservices.streamtheworld.com/api/livestream-redirect/VERONICAAAC.aac"},
  {"NPO-1", "https://www.mp3streams.nl/zender/npo-radio-1/stream/1-aac-64"},
  {"ERT Kosmos", "https://radiostreaming.ert.gr/ert-kosmos"},
  {"Real FM", "https://realfm.live24.gr/realfm"}
};

// Dynamic station list (loaded from storage)
RadioStation* stationList = nullptr;
int stationCount = 0;

// Button state
bool brightnessPressed = false;
bool nextStationPressed = false;
uint8_t brightnessLevel = 3;

// Volume control
int lastVolumePotValue = -1;

// Forward declarations
void playCustomStation(const char* name, const char* url);
void handleButtons();
void checkVolumeControl();
void loadStationsFromStorage();
void saveStationsToStorage();
void loadSettingsFromStorage();
void saveSettingsToStorage();
String getStationsJSON();

void setup() {
  Serial.begin(9600);
  delay(1000);
  
  Serial.println("\n");
  Serial.println("====================================");
  Serial.println("   ESP32 Internet Radio Clock");
  Serial.println("====================================");
  Serial.println();
  
  // Print memory info
  Serial.println("=== Memory Info ===");
  Serial.printf("Total heap: %d bytes\n", ESP.getHeapSize());
  Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
  Serial.printf("Total PSRAM: %d bytes\n", ESP.getPsramSize());
  Serial.printf("Free PSRAM: %d bytes\n", ESP.getFreePsram());
  
  // Print partition info
  Serial.println("\n=== Partition Info ===");
  const esp_partition_t* partition = esp_ota_get_running_partition();
  Serial.printf("Running partition: %s\n", partition->label);
  Serial.printf("Partition type: %d, subtype: %d\n", partition->type, partition->subtype);
  Serial.printf("Partition offset: 0x%X\n", partition->address);
  Serial.printf("Partition size: %d bytes (%.2f MB)\n", 
    partition->size, 
    partition->size / 1024.0 / 1024.0);
  
  Serial.println();
  
  // Initialize LED first for status indication
  if (ENABLE_LED) {
    led = new LEDModule(LED_PIN);
    led->begin();
    led->setColor(LEDModule::COLOR_RED, BRIGHT_DIM);
  }
  
  // Initialize Storage
  storage = new StorageModule();
  if (!storage->begin()) {
    Serial.println("ERROR: Storage initialization failed!");
    if (led) led->setColor(LEDModule::COLOR_RED, BRIGHT_FULL);
    delay(5000);
  } else {
    Serial.println("Storage initialized successfully");
    
    // Print storage info
    Serial.printf("Total space: %d KB\n", storage->getTotalSpace() / 1024);
    Serial.printf("Used space: %d KB\n", storage->getUsedSpace() / 1024);
    Serial.printf("Free space: %d KB\n", storage->getFreeSpace() / 1024);
    Serial.println();
  }
  
  // Load WiFi credentials from storage
  WiFiCredentials wifiCreds;
  bool hasStoredWifi = false;
  
  if (storage && storage->loadWiFiCredentials(wifiCreds)) {
    Serial.println("Using stored WiFi credentials");
    wifi = new WiFiModule(wifiCreds.ssid, wifiCreds.password);
    hasStoredWifi = true;
  } else {
    Serial.println("Using default WiFi credentials");
    wifi = new WiFiModule(WIFI_SSID, WIFI_PASSWORD);
    
    // Save default credentials to storage for next time
    if (storage) {
      storage->saveWiFiCredentials(WIFI_SSID, WIFI_PASSWORD);
      Serial.println("Saved default WiFi credentials to storage");
    }
  }
  
  // Connect to WiFi
  if (!wifi->connect()) {
    Serial.println("ERROR: WiFi connection failed!");
    if (led) led->setColor(LEDModule::COLOR_RED, BRIGHT_FULL);
  } else {
    if (led) led->setColor(LEDModule::COLOR_GREEN, BRIGHT_DIM);
  }
  
  // Initialize Web Server
  if (ENABLE_WEB && wifi->isConnected()) {
    webServer = new WebServerModule();
    webServer->setPlayCallback(playCustomStation);
    webServer->begin(MDNS_NAME);
    
    Serial.printf("Access web interface at:\n");
    Serial.printf("  http://%s.local\n", MDNS_NAME);
    Serial.printf("  http://%s\n", wifi->getLocalIP().c_str());
    Serial.println();
  }
  
  // Initialize Time Module
  int gmtOffset = GMT_OFFSET;
  int dstOffset = DST_OFFSET;
  
  if (storage && storage->loadTimezone(gmtOffset, dstOffset)) {
    Serial.printf("Loaded timezone from storage: GMT%+d, DST=%d\n", gmtOffset, dstOffset);
  }
  
  timeModule = new TimeModule(gmtOffset, dstOffset);
  timeModule->begin();
  
  // Initialize Display
  Serial.println("=== Display Init ===");
  
#ifdef USE_OLED_DISPLAY
  Serial.println("Initializing OLED display...");
  display = new DisplayOLED(I2C_SDA, I2C_SCL);
#elif defined(USE_TFT_DISPLAY)
  Serial.println("Initializing TFT display...");
  display = new DisplayTFT();  // TFT_eSPI uses pins from User_Setup.h
#endif
  
  if (display) {
    if (display->begin()) {
      Serial.println("Display initialized successfully");
      display->showStartupMessage("Radio Clock");
      delay(2000);
      
      // Load brightness from storage
      if (storage) {
        brightnessLevel = storage->loadBrightness(3);
        display->setBrightness(brightnessLevel);
      }
    } else {
      Serial.println("ERROR: Display initialization failed!");
    }
  } else {
    Serial.println("No display configured");
  }
  
  Serial.println();
  
  // Load stations from storage or use defaults
  loadStationsFromStorage();
  
  // Initialize Audio
  if (ENABLE_AUDIO) {
    Serial.println("=== Audio Init ===");
    #if ENABLE_STEREO
      Serial.println("Stereo mode: 2 channels");
    #else
      Serial.println("Mono mode: 1 channel");
    #endif
    
    audio = new AudioModule(I2S_BCLK, I2S_LRC, I2S_DOUT_L, MAX_VOLUME);
    audio->begin();
    audio->setStationList(stationList, stationCount);
    
    // Load and set volume from storage
    if (storage) {
      int savedVolume = storage->loadVolume(10);
      audio->setVolume(savedVolume);
      Serial.printf("Loaded volume from storage: %d\n", savedVolume);
    }
    
    // Start playing first station
    if (stationCount > 0) {
      audio->playStation(0);
    }
    
    Serial.println();
  }
  
  // Initialize FM Radio (only if enabled)
  #if ENABLE_FM_RADIO
  if (ENABLE_FM_RADIO) {
    Serial.println("=== FM Radio Init ===");
    fmRadio = new FMRadioModule(FM_RESET_PIN);
    
    if (fmRadio->begin()) {
      Serial.println("FM Radio initialized");
      fmRadio->setFM();  // Start in FM mode
      fmRadio->setFrequency(9580);  // Default: 95.8 MHz
      fmRadio->enableRDS();
      
      pinMode(AUDIO_SWITCH_PIN, OUTPUT);
      digitalWrite(AUDIO_SWITCH_PIN, LOW);  // Start with internet radio
      currentAudioSource = SOURCE_INTERNET_RADIO;
    } else {
      Serial.println("FM Radio initialization failed");
    }
    
    Serial.println();
  }
  #else
    Serial.println("=== FM Radio Disabled ===");
    Serial.println("Set ENABLE_FM_RADIO to true in config.h when module arrives");
    Serial.println();
  #endif
  
  // Setup button pins
  pinMode(BRIGHTNESS_PIN, INPUT);
  pinMode(NEXT_STATION_PIN, INPUT);
  pinMode(VOL_PIN, INPUT);
  
  Serial.println("====================================");
  Serial.println("   Setup Complete - System Ready");
  Serial.println("====================================");
  Serial.println();
  
  if (led) led->setColor(LEDModule::COLOR_BLUE, BRIGHT_DIM);
}

void loop() {
  // Check WiFi connection
  if (wifi) {
    wifi->checkConnection();
  }
  
  // Handle web requests
  if (webServer) {
    webServer->handleClient();
  }
  
  // Update time
  if (timeModule) {
    timeModule->loop();
  }
  
  // Update display based on current mode
  if (display && timeModule) {
    #if ENABLE_FM_RADIO
    if (currentAudioSource == SOURCE_INTERNET_RADIO) {
    #else
    // FM Radio disabled, always show internet radio or clock
    if (true) {
    #endif
      // Check if we're actively playing or in clock mode
      if (audio && audio->getCurrentStationName() != "Unknown") {
        // Internet radio is playing - show station info
        display->setMode(MODE_INTERNET_RADIO);
        display->showInternetRadio(
          audio->getCurrentStationName().c_str(),
          nullptr,  // TODO: Get metadata from audio stream
          nullptr
        );
      } else {
        // No station playing - show clock
        display->setMode(MODE_CLOCK);
        display->drawClock(
          timeModule->getHours(), 
          timeModule->getMinutes(), 
          timeModule->getSeconds()
        );
      }
    #if ENABLE_FM_RADIO
    } else if (currentAudioSource == SOURCE_FM_RADIO && fmRadio) {
      // FM radio is active - show FM info
      display->setMode(MODE_FM_RADIO);
      display->showFMRadio(
        fmRadio->getFrequencyString().c_str(),
        fmRadio->getRDSStationName(),
        fmRadio->getRDSText()
      );
    #endif
    } else {
      // Fallback to clock mode
      display->setMode(MODE_CLOCK);
      display->drawClock(
        timeModule->getHours(), 
        timeModule->getMinutes(), 
        timeModule->getSeconds()
      );
    }
    
    // Show small status info
    int vol = audio ? audio->getCurrentVolume() : 0;
    display->drawInfo(vol, brightnessLevel);
    display->update();
  }
  
  // Audio loop
  if (audio) {
    audio->loop();
  }
  
  // FM Radio loop (check for RDS updates) - only if enabled
  #if ENABLE_FM_RADIO
  if (fmRadio && currentAudioSource == SOURCE_FM_RADIO) {
    if (fmRadio->checkRDS()) {
      // RDS data updated, display will refresh automatically
      Serial.printf("RDS: %s - %s\n", 
        fmRadio->getRDSStationName(), 
        fmRadio->getRDSText()
      );
    }
  }
  #endif
  
  // Handle buttons
  handleButtons();
  
  // Handle volume control
  checkVolumeControl();
  
  delay(1);
}

// Callback for web interface to play custom station
void playCustomStation(const char* name, const char* url) {
  Serial.println("\n=== Playing Custom Station ===");
  Serial.printf("Name: %s\n", name);
  Serial.printf("URL: %s\n", url);
  
  if (audio) {
    audio->playCustom(name, url);
    
    // Optionally save this as a new preset
    if (storage && stationCount < MAX_STATIONS) {
      // Find if station already exists
      bool exists = false;
      for (int i = 0; i < stationCount; i++) {
        if (strcmp(stationList[i].url.c_str(), url) == 0) {
          exists = true;
          break;
        }
      }
      
      // Add new station if it doesn't exist
      if (!exists) {
        storage->saveStation(stationCount, name, url);
        stationCount++;
        storage->setStationCount(stationCount);
        
        // Reload station list
        loadStationsFromStorage();
        if (audio) {
          audio->setStationList(stationList, stationCount);
        }
        
        Serial.println("Station saved to presets");
      }
    }
  }
}

void handleButtons() {
  // Brightness button
  int brightnessButton = digitalRead(BRIGHTNESS_PIN);
  
  if (brightnessButton == HIGH && !brightnessPressed) {
    brightnessPressed = true;
    brightnessLevel = (brightnessLevel + 1) % 6;
    
    if (display) {
      display->setBrightness(brightnessLevel);
    }
    
    // Save brightness to storage
    if (storage) {
      storage->saveBrightness(brightnessLevel);
    }
    
    Serial.printf("Brightness changed to: %d\n", brightnessLevel);
    
  } else if (brightnessButton == LOW) {
    brightnessPressed = false;
  }
  
  // Next station button
  int nextButton = digitalRead(NEXT_STATION_PIN);
  
  if (nextButton == HIGH && !nextStationPressed) {
    nextStationPressed = true;
    
    if (audio) {
      audio->nextStation();
      Serial.printf("Playing: %s\n", audio->getCurrentStationName().c_str());
    }
    
  } else if (nextButton == LOW) {
    nextStationPressed = false;
  }
}

void checkVolumeControl() {
  if (!audio) return;
  
  int potValue = analogRead(VOL_PIN);
  
  // Only update if change is significant (reduce noise/jitter)
  if (lastVolumePotValue == -1 || abs(potValue - lastVolumePotValue) > 40) {
    lastVolumePotValue = potValue;
    
    int newVolume = map(potValue, 0, 4095, 0, MAX_VOLUME);
    int currentVolume = audio->getCurrentVolume();
    
    if (abs(newVolume - currentVolume) >= 1) {
      audio->setVolume(newVolume);
      
      // Save volume to storage (but not too frequently)
      static unsigned long lastVolumeSave = 0;
      if (millis() - lastVolumeSave > 2000) {  // Save at most every 2 seconds
        if (storage) {
          storage->saveVolume(newVolume);
        }
        lastVolumeSave = millis();
      }
    }
  }
}

void loadStationsFromStorage() {
  Serial.println("=== Loading Stations ===");
  
  if (!storage) {
    // Use default stations
    stationCount = sizeof(defaultStations) / sizeof(RadioStation);
    stationList = defaultStations;
    Serial.printf("Using %d default stations\n", stationCount);
    return;
  }
  
  int storedCount = storage->getStationCount();
  
  if (storedCount > 0) {
    // Allocate memory for stored stations
    stationList = new RadioStation[storedCount];
    stationCount = 0;
    
    // Load each station
    for (int i = 0; i < storedCount; i++) {
      StoredStation stored;
      if (storage->loadStation(i, stored)) {
        stationList[stationCount].name = String(stored.name);
        stationList[stationCount].url = String(stored.url);
        stationCount++;
        Serial.printf("Loaded station %d: %s\n", i, stored.name);
      }
    }
    
    Serial.printf("Loaded %d stations from storage\n", stationCount);
  } else {
    // First time setup - save default stations to storage
    Serial.println("No stored stations found, saving defaults...");
    
    int defaultCount = sizeof(defaultStations) / sizeof(RadioStation);
    for (int i = 0; i < defaultCount; i++) {
      storage->saveStation(i, 
        defaultStations[i].name.c_str(), 
        defaultStations[i].url.c_str()
      );
    }
    storage->setStationCount(defaultCount);
    
    // Use default stations
    stationList = defaultStations;
    stationCount = defaultCount;
    
    Serial.printf("Saved and using %d default stations\n", defaultCount);
  }
  
  Serial.println();
}

void saveStationsToStorage() {
  if (!storage || !stationList) {
    return;
  }
  
  Serial.println("=== Saving Stations ===");
  
  for (int i = 0; i < stationCount; i++) {
    storage->saveStation(i, 
      stationList[i].name.c_str(), 
      stationList[i].url.c_str()
    );
  }
  
  storage->setStationCount(stationCount);
  Serial.printf("Saved %d stations to storage\n", stationCount);
}

// Helper function to get stations as JSON for web interface
String getStationsJSON() {
  String json = "{\"stations\":[";
  
  for (int i = 0; i < stationCount; i++) {
    if (i > 0) json += ",";
    json += "{\"name\":\"" + stationList[i].name + "\",";
    json += "\"url\":\"" + stationList[i].url + "\"}";
  }
  
  json += "]}";
  return json;
}

/*
 * USAGE NOTES:
 * 
 * 1. First Upload:
 *    - WiFi credentials are saved to storage
 *    - Default stations are saved to storage
 *    - Default settings (volume, brightness) are saved
 * 
 * 2. Subsequent Boots:
 *    - System loads WiFi credentials from storage
 *    - System loads stations from storage
 *    - System loads settings from storage
 * 
 * 3. Adding New Stations:
 *    - Use web interface to play a custom URL
 *    - Station is automatically saved to storage
 *    - Will be available after reboot
 * 
 * 4. Resetting to Defaults:
 *    - Uncomment this line in setup(): storage->format();
 *    - Upload and run once
 *    - Re-comment the line and upload again
 * 
 * 5. Storage Locations:
 *    - WiFi: NVS (Preferences)
 *    - Stations: NVS (Preferences)
 *    - Settings: NVS (Preferences)
 *    - WAV files: LittleFS (for future alarm sounds)
 * 
 * 6. Memory Management:
 *    - Station list uses dynamic allocation when loaded from storage
 *    - Default station list uses static allocation
 *    - All storage operations are non-blocking
 */