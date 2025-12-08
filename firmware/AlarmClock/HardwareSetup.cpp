#include "HardwareSetup.h"

HardwareSetup::HardwareSetup() 
    : display(nullptr), timeModule(nullptr), fmRadio(nullptr), 
      storage(nullptr), wifi(nullptr), 
      audio(nullptr), webServer(nullptr), led(nullptr),
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
    Serial.println("HW - Init WebServer");
    initWebServer();
    Serial.println("HW - Init Audio");
    initAudio();
    Serial.println("HW - Init FMRadio");
    initFMRadio();
    Serial.println("HW - Init Done");
    
    delay(2000);
    if (display) {
        display->clear();
        display->drawClockFace();
    }
    
    if (led) led->setColor(LEDModule::COLOR_BLUE, BRIGHT_DIM);
    
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
    display = new DisplayILI9341(TFT_CS, TFT_DC, TFT_RST, TFT_MOSI, TFT_SCLK, TFT_MISO, TFT_BL);
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
        
        // IMPORTANT: Set all required modules BEFORE calling begin()
        if (storage) {
            webServer->setStorageModule(storage);
        }
        if (timeModule) {
            webServer->setTimeModule(timeModule);
        }
        if (audio) {
            webServer->setAudioModule(audio);
        }
        if (fmRadio) {
            webServer->setFMRadioModule(fmRadio);
        }
        
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
        audio->begin();
        audio->setVolume(3);
        if (display) display->drawText(10, 140, "Audio: OK", ILI9341_GREEN, 2);
    }
}

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
