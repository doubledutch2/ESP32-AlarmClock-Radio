#include "HardwareSetup.h"

HardwareSetup::HardwareSetup() 
    : display(nullptr), timeModule(nullptr), fmRadio(nullptr), 
      storage(nullptr), wifi(nullptr), 
      audio(nullptr), webServer(nullptr), led(nullptr), touchScreen(nullptr),
      lastVolumePotValue(-1), brightnessLevel(3) {
}

HardwareSetup::~HardwareSetup() {
    if (display) delete display;
    if (timeModule) delete timeModule;
    if (fmRadio) delete fmRadio;
    if (storage) delete storage;
    if (wifi) delete wifi;
    if (audio) delete audio;
    if (webServer) delete webServer;
    if (led) delete led;
    if (touchScreen) delete touchScreen;
}

bool HardwareSetup::begin() {
    Serial.println("HW - Init Display");
    initDisplay();
    
    Serial.println("HW - Init WiFi");
    initWiFi();
    
    Serial.println("HW - Init Buttons");
    initButtons();
    
    Serial.println("HW - Init LED");
    initLED();
    
    Serial.println("HW - Init Storage");
    initStorage();
    
    Serial.println("HW - Init Time");
    initTime();
    
    Serial.println("HW - Init Audio (this may take a moment...)");
    initAudio();
    Serial.println("HW - Audio init completed");
    
    Serial.println("HW - Init FMRadio");
    initFMRadio();
    
    Serial.println("HW - Init WebServer");
    initWebServer();
    
    Serial.println("HW - Init TouchScreen");
    initTouchScreen();
    
    Serial.println("HW - Init Done");
    Serial.println("===========================================");
    Serial.println("Preparing to clear display and show clock...");
    
    // Give user time to see init messages
    delay(1000);
    
    // FORCE clear the display multiple times if needed
    if (display) {
        Serial.println("Step 1: Clearing display...");
        display->clear();
        delay(100);
        
        Serial.println("Step 2: Drawing clock face...");
        display->drawClockFace();
        delay(100);
        
        Serial.println("Step 3: Display should now show clock");
    } else {
        Serial.println("ERROR: Display is null!");
    }
    
    if (led) {
        led->setColor(LEDModule::COLOR_BLUE, BRIGHT_DIM);
    }
    
    Serial.println("===========================================");
    Serial.println("Hardware initialization complete!");
    Serial.println("===========================================");
    
    return true;
}

void HardwareSetup::loop() {
    // WiFi maintenance
    if (wifi) wifi->checkConnection();
    
    // Web server
    if (webServer) webServer->handleClient();
    
    // Audio streaming
    if (audio) audio->loop();
    
    // Hardware controls
    handleVolumeControl();
    handleBrightnessButton();
    handleNextStationButton();
}

void HardwareSetup::initButtons() {
    pinMode(BTN_UP, INPUT_PULLUP);
    pinMode(BTN_DOWN, INPUT_PULLUP);
    pinMode(BTN_SELECT, INPUT_PULLUP);
    pinMode(BTN_SNOOZE, INPUT_PULLUP);
    pinMode(BRIGHTNESS_PIN, INPUT);
    pinMode(NEXT_STATION_PIN, INPUT);
    pinMode(VOL_PIN, INPUT);
}

void HardwareSetup::initDisplay() {
    Serial.println("Initializing Display...");
    // display = new DisplayILI9341(TFT_CS, TFT_DC, TFT_RST, TFT_MOSI, TFT_SCLK, TFT_MISO, TFT_BL);
    display = new DisplayILI9341(TFT_CS, TFT_DC, -1, TFT_MOSI, TFT_SCLK, TFT_MISO, TFT_BL);
    display->begin();
    display->setBrightness(200);
    display->clear();
    display->drawText(10, 10, "Alarm Clock Starting...", ILI9341_WHITE, 2);
}

void HardwareSetup::initStorage() {
    Serial.println("Initializing Storage...");
    storage = new StorageModule();
    if (storage->begin()) {
        if (display) display->drawText(10, 40, "Storage: OK", ILI9341_GREEN, 2);
        if (led) led->setColor(LEDModule::COLOR_YELLOW, BRIGHT_DIM);
    } else {
        if (display) display->drawText(10, 40, "Storage: FAIL", ILI9341_RED, 2);
    }
}

void HardwareSetup::initWiFi() {
    Serial.println("Initializing WiFi...");
    if (display) display->drawText(10, 60, "Connecting WiFi...", ILI9341_YELLOW, 2);    
    wifi = new WiFiModule(WIFI_SSID, WIFI_PASSWORD);
    if (wifi->connect()) {
        Serial.println("WiFi Connected!");
        if (display) {
            display->drawText(10, 60, "WiFi: OK", ILI9341_GREEN, 2);
            display->drawText(10, 80, wifi->getLocalIP().c_str(), ILI9341_CYAN, 1);
        }
        Serial.println("Set LED to Green");
        if (led) led->setColor(LEDModule::COLOR_GREEN, BRIGHT_DIM);
    } else {
        Serial.println("WiFi Connection Failed!");
        if (display) display->drawText(10, 60, "WiFi: FAIL", ILI9341_RED, 2);
        Serial.println("Set LED to Red");
        if (led) led->setColor(LEDModule::COLOR_RED, BRIGHT_FULL);
    }
    Serial.println("WiFi Done");
}

void HardwareSetup::initTime() {
    Serial.println("Initializing Time...");
    
    // Load timezone settings from storage if available
    long gmtOffset = DEFAULT_GMT_OFFSET_SEC;
    long dstOffset = DEFAULT_DAYLIGHT_OFFSET_SEC;
    
    if (storage && storage->isReady()) {
        storage->loadTimezone(gmtOffset, dstOffset);
        Serial.printf("Using stored timezone: GMT %ld, DST %d\n", gmtOffset, dstOffset);
    } else {
        Serial.println("Using default timezone settings");
    }
    
    timeModule = new TimeModule("pool.ntp.org", gmtOffset, dstOffset);
    if (timeModule->begin(WIFI_SSID, WIFI_PASSWORD)) {
        if (display) display->drawText(10, 100, "Time: OK", ILI9341_GREEN, 2);
    } else {
        if (display) display->drawText(10, 100, "Time: FAIL", ILI9341_RED, 2);
    }
}

void HardwareSetup::initWebServer() {
    if (ENABLE_WEB && wifi && wifi->isConnected()) {
        webServer = new WebServerModule();
        
        // CRITICAL: Set ALL modules BEFORE calling begin()
        if (storage) {
            webServer->setStorageModule(storage);
            Serial.println("WebServer: Storage module set");
        }
        if (timeModule) {
            webServer->setTimeModule(timeModule);
            Serial.println("WebServer: Time module set");
        }
        if (audio) {
            webServer->setAudioModule(audio);
            Serial.println("WebServer: Audio module set");
        }
        if (fmRadio) {
            webServer->setFMRadioModule(fmRadio);
            Serial.println("WebServer: FM Radio module set");
        }
        
        // Now call begin() - this will set up WebServerAlarms routes
        webServer->begin(MDNS_NAME);
        
        if (display) display->drawText(10, 120, "Web: OK", ILI9341_GREEN, 2);
        Serial.printf("Web interface: http://%s.local or http://%s\n", 
                     MDNS_NAME, wifi->getLocalIP().c_str());
    }
}

void HardwareSetup::initAudio() {
    if (ENABLE_AUDIO) {
        Serial.println("Initializing Audio...");
        audio = new AudioModule(I2S_BCLK, I2S_LRC, I2S_DOUT, MAX_VOLUME);
        
        if (audio) {
            Serial.println("AudioModule created, calling begin()...");
            audio->begin();
            audio->setVolume(3);
            Serial.println("AudioModule initialization complete");
            if (display) display->drawText(10, 140, "Audio: OK", ILI9341_GREEN, 2);
        } else {
            Serial.println("Failed to create AudioModule");
            if (display) display->drawText(10, 140, "Audio: FAIL", ILI9341_RED, 2);
        }
    } else {
        Serial.println("Audio disabled in config");
    }
}// Update begin() - Force display clear and add better error handling

void HardwareSetup::initFMRadio() {
    if (ENABLE_FM_RADIO) {
        Serial.println("Initializing FM Radio...");
        Wire.begin(I2C_SDA, I2C_SCL);
        delay(100);
        fmRadio = new FMRadioModule();
        if (fmRadio->begin()) {
            Serial.println("FM Radio initialized");
            if (display) display->drawText(10, 160, "FM Radio: OK", ILI9341_GREEN, 2);
        } else {
            Serial.println("FM Radio initialization failed!");
            if (display) display->drawText(10, 160, "FM Radio: FAIL", ILI9341_RED, 2);
        }
    }
}

void HardwareSetup::initLED() {
    if (ENABLE_LED) {
        led = new LEDModule(LED_PIN);
        led->begin();
        led->setColor(LEDModule::COLOR_RED, BRIGHT_DIM);
    }
}

void HardwareSetup::handleVolumeControl() {
    if (!audio) return;
    
    int potValue = analogRead(VOL_PIN);
    
    if (lastVolumePotValue == -1 || abs(potValue - lastVolumePotValue) > 40) {
        lastVolumePotValue = potValue;
        int newVolume = map(potValue, 0, 4095, 0, audio->getMaxVolume());
        audio->setVolume(newVolume);
    }
}

void HardwareSetup::handleBrightnessButton() {
    static bool brightnessPressed = false;
    int brightnessButton = digitalRead(BRIGHTNESS_PIN);
    
    if (brightnessButton == HIGH && !brightnessPressed) {
        brightnessPressed = true;
        brightnessLevel = (brightnessLevel + 1) % 6;
        if (display) display->setBrightness(brightnessLevel * 50);
        Serial.printf("Brightness: %d\n", brightnessLevel);
    } else if (brightnessButton == LOW) {
        brightnessPressed = false;
    }
}

void HardwareSetup::handleNextStationButton() {
    static bool nextStationPressed = false;
    int nextButton = digitalRead(NEXT_STATION_PIN);
    
    if (nextButton == HIGH && !nextStationPressed) {
        nextStationPressed = true;
        if (audio) {
            audio->nextStation();
            Serial.printf("Next station: %s\n", audio->getCurrentStationName().c_str());
        }
    } else if (nextButton == LOW) {
        nextStationPressed = false;
    }
}

void HardwareSetup::initTouchScreen() {
    #if ENABLE_TOUCHSCREEN
        Serial.println("TouchScreen: Creating module using TFT_eSPI...");
        
        if (!display) {
            Serial.println("ERROR: Display must be initialized before touchscreen");
            touchScreen = nullptr;
            return;
        }
        
        // Pass the TFT_eSPI object from display to touchscreen
        touchScreen = new TouchScreenModule(display->getTFT());
        
        if (touchScreen) {
            bool success = touchScreen->begin();
            if (success) {
                Serial.println("TouchScreen: Initialized successfully (TFT_eSPI built-in)");
            } else {
                Serial.println("WARNING: TouchScreen initialization failed");
                delete touchScreen;
                touchScreen = nullptr;
            }
        } else {
            Serial.println("ERROR: Failed to create TouchScreenModule");
        }
    #else
        Serial.println("TouchScreen disabled in config (ENABLE_TOUCHSCREEN = false)");
        touchScreen = nullptr;
    #endif
}