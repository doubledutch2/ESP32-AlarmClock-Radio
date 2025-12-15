#ifndef HARDWARE_SETUP_H
#define HARDWARE_SETUP_H

#include "Config.h"
#include "DisplayILI9341.h"
#include "TimeModule.h"
#include "FMRadioModule.h"
#include "StorageModule.h"
#include "WiFiModule.h"
#include "AudioModule.h"
#include "WebServerModule.h"
#include "LEDModule.h"
#include "TouchScreenModule.h"  // NEW
#include <SPI.h>

class HardwareSetup {
private:
    DisplayILI9341* display;
    TimeModule* timeModule;
    FMRadioModule* fmRadio;
    StorageModule* storage;
    WiFiModule* wifi;
    AudioModule* audio;
    WebServerModule* webServer;
    LEDModule* led;
    TouchScreenModule* touchScreen;  // NEW
    
    int lastVolumePotValue;
    uint8_t brightnessLevel;
    int fontHeight = 20;
    int lastRow     = 60;

public:
    HardwareSetup();
    ~HardwareSetup();
    
    bool begin();
    void loop();
    
    // Getters
    DisplayILI9341* getDisplay() { return display; }
    TimeModule* getTimeModule() { return timeModule; }
    FMRadioModule* getFMRadio() { return fmRadio; }
    StorageModule* getStorage() { return storage; }
    WiFiModule* getWiFi() { return wifi; }
    AudioModule* getAudio() { return audio; }
    WebServerModule* getWebServer() { return webServer; }
    LEDModule* getLED() { return led; }
    TouchScreenModule* getTouchScreen() { return touchScreen; }  // NEW
    
private:
    void initButtons();
    void initDisplay();
    void initStorage();
    void initWiFi();
    void initTime();
    void initWebServer();
    void initAudio();
    void initFMRadio();
    void initLED();
    void initTouchScreen();  // NEW
    void doI2CScan();
    
    void handleVolumeControl();
    void handleBrightnessButton();
    void handleNextStationButton();
};

#endif