#ifndef HARDWARE_SETUP_H
#define HARDWARE_SETUP_H

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

class HardwareSetup {
private:
    DisplayILI9341* display;
    TimeModule* timeModule;
    FMRadioModule* fmRadio;
    BuzzerModule* buzzer;
    StorageModule* storage;
    WiFiModule* wifi;
    AudioModule* audio;
    WebServerModule* webServer;
    LEDModule* led;
    
    int lastVolumePotValue;
    uint8_t brightnessLevel;

public:
    HardwareSetup();
    ~HardwareSetup();
    
    bool begin();
    void loop();
    
    // Getters
    DisplayILI9341* getDisplay() { return display; }
    TimeModule* getTimeModule() { return timeModule; }
    FMRadioModule* getFMRadio() { return fmRadio; }
    BuzzerModule* getBuzzer() { return buzzer; }
    StorageModule* getStorage() { return storage; }
    WiFiModule* getWiFi() { return wifi; }
    AudioModule* getAudio() { return audio; }
    WebServerModule* getWebServer() { return webServer; }
    LEDModule* getLED() { return led; }
    
private:
    void initButtons();
    void initDisplay();
    void initStorage();
    void initWiFi();
    void initTime();
    void initWebServer();
    void initAudio();
    void initFMRadio();
    void initBuzzer();
    void initLED();
    
    void handleVolumeControl();
    void handleBrightnessButton();
    void handleNextStationButton();
};

#endif
