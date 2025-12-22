#include "HardwareSetup.h"
#include "FeatureFlags.h"
#include "StorageModule.h"  // ADD THIS LINE EXPLICITLY

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
    lastRow = 25;

    Serial.println("HW - Init Display");
    initDisplay();
    
    Serial.println("HW - Init WiFi");
    initWiFi();

    Serial.println("HW - Init Storage");
    initStorage();
    
    // Load saved settings after storage is initialized
    loadSavedSettings();
/**/    
    Serial.println("HW - Init Buttons");
    initButtons();
    
    Serial.println("HW - Init LED");
    initLED();
/**/    
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

    Serial.println("HW - I2C Scan");
    doI2CScan();

    Serial.println("HW - Init Done");
    Serial.println("===========================================");
    Serial.println("Preparing to clear display and show clock...");
    if (display) display->drawText(10, lastRow, "Init: DONE!", ILI9341_WHITE, 1);

    // Give user time to see init messages
    delay(3000);
    
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
        // led->setColor(LEDModule::COLOR_BLUE, BRIGHT_DIM);
    }

    Serial.println("===========================================");
    Serial.println("Hardware initialization complete!");
    Serial.println("===========================================");
    
    return true;
}

void HardwareSetup::loadSavedSettings() {
    if (!storage || !storage->isReady()) {
        Serial.println("Storage not ready, using defaults");
        return;
    }
    
    Serial.println("Loading saved settings from NVS...");
    
    storage->loadFeatureFlags(activeFlags); 
    
    // Load volume
    uint8_t savedVolume = storage->loadVolume(3);
    if (audio) {
        audio->setVolume(savedVolume);
        Serial.printf("Volume set to: %d\n", savedVolume);
    }
    
    // Load brightness
    uint8_t savedBrightness = storage->loadBrightness(200);
    if (display) {
        display->setBrightness(savedBrightness);
        Serial.printf("Brightness set to: %d\n", savedBrightness);
    }
    
    // Load feature flags
    FeatureFlags flags;
    storage->loadFeatureFlags(flags);
    Serial.println("Feature flags loaded:");
    Serial.printf("  TouchScreen: %s\n", flags.enableTouchScreen ? "ON" : "OFF");
    Serial.printf("  Buttons: %s\n", flags.enableButtons ? "ON" : "OFF");
    Serial.printf("  Draw: %s\n", flags.enableDraw ? "ON" : "OFF");
    Serial.printf("  Audio: %s\n", flags.enableAudio ? "ON" : "OFF");
    Serial.printf("  LED: %s\n", flags.enableLED ? "ON" : "OFF");
    Serial.printf("  Alarms: %s\n", flags.enableAlarms ? "ON" : "OFF");
    Serial.printf("  FM Radio: %s\n", flags.enableFMRadio ? "ON" : "OFF");
    
    // Note: Feature flags affect hardware initialization which happens in Config.h
    // These are loaded here for display but require restart to take effect
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
    display = new DisplayILI9341(TFT_CS, TFT_DC, -1, TFT_MOSI, TFT_SCLK, TFT_MISO, TFT_BL);
    display->begin();
    display->setBrightness(200);
    display->clear();
    display->drawText(10, 10, "Alarm Clock Starting", ILI9341_WHITE, 2);
    lastRow += fontHeight;
}

void HardwareSetup::initStorage() {
    Serial.println("Initializing Storage...");
    storage = new StorageModule();
    if (storage->begin()) {
        if (display) display->drawText(10, lastRow, "Storage: OK", ILI9341_WHITE, 1);
        if (led) led->setColor(LEDModule::COLOR_YELLOW, BRIGHT_DIM);
    } else {
        if (display) display->drawText(10, lastRow, "Storage: FAIL", ILI9341_RED, 1);
    }
    lastRow += fontHeight;
}

void HardwareSetup::initWiFi() {
    Serial.println("Initializing WiFi...");
    if (display) display->drawText(10, lastRow, "Connecting WiFi:", ILI9341_WHITE, 1);    
    wifi = new WiFiModule(WIFI_SSID, WIFI_PASSWORD);
    if (wifi->connect()) {
        Serial.println("WiFi Connected!");
        if (display) {
            display->drawText(120, lastRow, "OK - ", ILI9341_WHITE, 1);
            display->drawText(150, lastRow, wifi->getLocalIP().c_str(), ILI9341_WHITE, 1);

        }
        Serial.println("Set LED to Green");
        if (led) led->setColor(LEDModule::COLOR_GREEN, BRIGHT_DIM);
    } else {
        Serial.println("WiFi Connection Failed!");
        if (display) display->drawText(80, lastRow, "FAIL", ILI9341_RED, 1);
        Serial.println("Set LED to Red");
        if (led) led->setColor(LEDModule::COLOR_RED, BRIGHT_FULL);
    }
    Serial.println("WiFi Done");
    lastRow += fontHeight;
}

void HardwareSetup::initTime() {
    Serial.println("Initializing Time...");
    
    // With ezTime, we use timezone names instead of offsets
    // You can either auto-detect or specify a timezone
    
    // Option 1: Auto-detect timezone (recommended)
    timeModule = new TimeModule();
    
    // Option 2: Specify timezone (uncomment if you want to hardcode it)
    // timeModule = new TimeModule("Europe/London");  // UK
    // timeModule = new TimeModule("America/New_York");  // US East Coast
    
    if (timeModule->begin(WIFI_SSID, WIFI_PASSWORD)) {
        if (display) display->drawText(10, lastRow, "Time: OK", ILI9341_WHITE, 1);
        Serial.println("Timezone: " + timeModule->getTimezoneName());
        Serial.println("DST active: " + String(timeModule->isDST() ? "Yes" : "No"));
    } else {
        if (display) display->drawText(10, lastRow, "Time: FAIL", ILI9341_RED, 1);
    }
    lastRow += fontHeight;
}

void HardwareSetup::initWebServer() {
    if (activeFlags.enableWeb && wifi && wifi->isConnected()) {
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
        if (display) {
            webServer->setDisplayModule(display);
            Serial.println("WebServer: Display module set");
        }
        
        // Now call begin() - this will set up WebServerAlarms routes
        webServer->begin(MDNS_NAME);
        
        if (display) display->drawText(10, lastRow, "Web: OK", ILI9341_WHITE, 1);
        Serial.printf("Web interface: http://%s.local or http://%s\n", 
                     MDNS_NAME, wifi->getLocalIP().c_str());
    }
    else {
        if (display) display->drawText(10, lastRow, "Web: Disabled or no WiFi", ILI9341_WHITE, 1);
    }
    lastRow += fontHeight;
}

void HardwareSetup::initAudio() {
    if (activeFlags.enableAudio) {
        Serial.println("Initializing Audio...");
        
        // Load saved volume or use default
        uint8_t savedVolume = 3;
        if (storage && storage->isReady()) {
            savedVolume = storage->loadVolume(3);
        }
        
        audio = new AudioModule(I2S_BCLK, I2S_LRC, I2S_DOUT, MAX_VOLUME, savedVolume);
        
        if (audio) {
            Serial.println("AudioModule created, calling begin()...");
            audio->begin();
            Serial.printf("AudioModule initialization complete with volume: %d\n", savedVolume);
            if (display) display->drawText(10, lastRow, "Audio: OK", ILI9341_WHITE, 1);
        } else {
            Serial.println("Failed to create AudioModule");
            if (display) display->drawText(10, lastRow, "Audio: FAIL", ILI9341_RED, 1);
        }
    } else {
        if (display) display->drawText(10, lastRow, "Init Audio: Disabled", ILI9341_WHITE, 1);
        Serial.println("Audio disabled in config");
    }
    lastRow += fontHeight;
}

void HardwareSetup::doI2CScan() {
    if (activeFlags.enableI2CScan) {
        int  sizeBuf = 100;
        char displayBuf[100] = "";
        char hexChar[10];

        if (display) display->drawText(10, lastRow, "I2C Scan:", ILI9341_WHITE, 1);
        Wire.begin(I2C_SDA, I2C_SCL);
        delay(100);
        byte error, address;
        int nDevices;

        Serial.println("Scanning...");

        nDevices = 0;
        for(address = 1; address < 127; address++ ) {
            Wire.beginTransmission(address);
            error = Wire.endTransmission();

            if (error == 0) {
                if (nDevices == 0) {
                    Serial.print("I2C device found at address: ");
                }
                else {
                    Serial.print(", ");
                }
                
                sprintf(hexChar,"0x%02x",address);
                Serial.print("HexChar: ");
                Serial.println(hexChar);
                if (strlen(displayBuf) < sizeBuf - 5) {
                    if (nDevices > 0) {
                        strcat(displayBuf,", ");
                    }
                    strcat(displayBuf,hexChar);
                }
                nDevices++;
            }
        }  

        if (nDevices == 0) {
            Serial.println("No I2C devices found\n");
            if (display) display->drawText(10, lastRow, "I2C Scan: No Devices", ILI9341_WHITE, 1);
        }
        else {
            if (display) {
                display->drawText(10, lastRow, "I2C Scan:", ILI9341_WHITE, 1);
                display->drawText(85, lastRow,displayBuf,ILI9341_WHITE,1);
            }
            Serial.println(displayBuf);
        }
    }
    else {
        if (display) display->drawText(10, lastRow, "I2C Scan: Disabled", ILI9341_WHITE, 1);
    }
    lastRow += fontHeight;
}

void HardwareSetup::initFMRadio() {
    if (activeFlags.enableFMRadio) {
        Serial.println("Initializing FM Radio...");
        Wire.begin(I2C_SDA, I2C_SCL);
        delay(100);
        fmRadio = new FMRadioModule();
        if (fmRadio->begin()) {
            Serial.println("FM Radio initialized");
            if (display) display->drawText(10, lastRow, "FM Radio: OK", ILI9341_WHITE, 1);
        } else {
            Serial.println("FM Radio initialization failed!");
            if (display) display->drawText(10, lastRow, "FM Radio: FAIL", ILI9341_RED, 1);
        }
    } 
    else {
        if (display) display->drawText(10, lastRow, "FM Radio: Disabled", ILI9341_WHITE, 1);
    }
    lastRow += fontHeight;
}

void HardwareSetup::initLED() {
    if (activeFlags.enableLED) {
        if (display) display->drawText(10, lastRow, "Init LED: OK", ILI9341_WHITE, 1);
        led = new LEDModule(LED_PIN);
        led->begin();
        led->setColor(LEDModule::COLOR_RED, BRIGHT_DIM);
    }
    else {
        if (display) display->drawText(10, lastRow, "Init LED: Disabled", ILI9341_WHITE, 1);
    }
    lastRow += fontHeight;
}

void HardwareSetup::handleVolumeControl() {
    if (!audio) return;
    
    int potValue = analogRead(VOL_PIN);
    
    if (lastVolumePotValue == -1 || abs(potValue - lastVolumePotValue) > 40) {
        lastVolumePotValue = potValue;
        int newVolume = map(potValue, 0, 4095, 0, audio->getMaxVolume());
        audio->setVolume(newVolume);
        
        // Save to storage periodically (not on every change to avoid wear)
        static unsigned long lastSave = 0;
        if (millis() - lastSave > 5000 && storage) {  // Save every 5 seconds max
            storage->saveVolume(newVolume);
            lastSave = millis();
        }
    }
}

void HardwareSetup::handleBrightnessButton() {
    if (!activeFlags.enableButtons) {
        return;
    }
    static bool brightnessPressed = false;
    int brightnessButton = digitalRead(BRIGHTNESS_PIN);
    
    if (brightnessButton == HIGH && !brightnessPressed) {
        brightnessPressed = true;
        brightnessLevel = (brightnessLevel + 1) % 6;
        uint8_t newBrightness = brightnessLevel * 50;
        if (display) {
            display->setBrightness(newBrightness);
        }
        if (storage) {
            storage->saveBrightness(newBrightness);
        }
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
    if (activeFlags.enableTouchScreen) {
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
                if (display) display->drawText(10, lastRow, "Init Touch: OK", ILI9341_WHITE, 1);
                Serial.println("TouchScreen: Initialized successfully (TFT_eSPI built-in)");
            } else {
                if (display) display->drawText(10, lastRow, "Init Touch: Fail", ILI9341_WHITE, 1);
                Serial.println("WARNING: TouchScreen initialization failed");
                delete touchScreen;
                touchScreen = nullptr;
            }
        } else {
            if (display) display->drawText(10, lastRow, "Init Touch: can not create touch", ILI9341_WHITE, 1);
            Serial.println("ERROR: Failed to create TouchScreenModule");
        }
    }
    else {
        Serial.println("TouchScreen disabled in config (ENABLE_TOUCHSCREEN = false)");
        if (display) display->drawText(10, lastRow, "Init Touch: disabled", ILI9341_WHITE, 1);
        touchScreen = nullptr;
    }
    lastRow += fontHeight;
}