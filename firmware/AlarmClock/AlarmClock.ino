/*
 * ESP32 Alarm Clock Radio
 * Complete modular alarm clock with FM radio, WiFi/NTP time, TFT display, and internal storage
 */

#include <Wire.h>
#include "TimeModule.h"
#include "DisplayILI9341.h"
#include "FMRadioModule.h"
#include "BuzzerModule.h"
#include "StorageModule.h"
#include "Config.h"

// ===== WIFI CREDENTIALS =====
#define WIFI_SSID "DEBEER"
#define WIFI_PASSWORD "B@C&86j@gqW73g"

// Time zone offset in seconds (e.g., GMT+0 = 0, GMT+1 = 3600, GMT-5 = -18000)
// For UK (GMT): use 0 in winter, 3600 in summer (BST)
#define GMT_OFFSET_SEC 0
#define DAYLIGHT_OFFSET_SEC 3600  // Add 1 hour for daylight saving time


// ===== GLOBAL OBJECTS =====
TimeModule* timeModule = nullptr;
DisplayILI9341* display = nullptr;
FMRadioModule* fmRadio = nullptr;
BuzzerModule* buzzer = nullptr;
StorageModule* storage = nullptr;

bool    radioConnected = false;

// ===== ALARM STATE =====
struct AlarmState {
    uint8_t hour;
    uint8_t minute;
    bool enabled;
    bool triggered;
    bool snoozed;
    unsigned long snoozeTime;
};

AlarmState alarmState;

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

// ===== CONSTANTS =====
const unsigned long SNOOZE_DURATION = 5 * 60 * 1000;  // 5 minutes
const unsigned long DEBOUNCE_DELAY = 200;
const unsigned long DISPLAY_UPDATE_INTERVAL = 1000;

// ===== SETUP =====
void setup() {
    Serial.begin(115200);
    Serial.println("\n\nESP32 Alarm Clock Radio Starting...");
    
    // Initialize Button Pins
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
    
    // Initialize WiFi and Time
    Serial.println("Initializing WiFi and NTP...");
    display->drawText(10, 40, "Connecting WiFi...", ILI9341_YELLOW, 2);
    timeModule = new TimeModule("pool.ntp.org", GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC);
    if (timeModule->begin(WIFI_SSID, WIFI_PASSWORD)) {
        Serial.println("WiFi and Time initialized successfully");
        display->drawText(10, 40, "WiFi: OK", ILI9341_GREEN, 2);
        display->drawText(10, 60, timeModule->getIPAddress().c_str(), ILI9341_CYAN, 1);
    } else {
        Serial.println("WiFi/Time initialization failed!");
        display->drawText(10, 40, "WiFi: FAIL", ILI9341_RED, 2);
        display->drawText(10, 60, "Check credentials", ILI9341_YELLOW, 1);
    }
    
    // Initialize FM Radio
    Serial.println("Initializing FM Radio...");
    Wire.begin(I2C_SDA, I2C_SCL);
    delay(100);
    fmRadio = new FMRadioModule();
    if (fmRadio->begin()) {
        radioConnected = true;
        Serial.println("FM Radio initialized successfully");
        display->drawText(10, 90, "FM Radio: OK", ILI9341_GREEN, 2);
    } else {
        Serial.println("FM Radio initialization failed!");
        display->drawText(10, 90, "FM Radio: FAIL", ILI9341_RED, 2);
    }
    
    // Initialize Buzzer
    Serial.println("Initializing Buzzer...");
    buzzer = new BuzzerModule(BUZZER_PIN);
    buzzer->begin();
    buzzer->playBeep();
    display->drawText(10, 120, "Buzzer: OK", ILI9341_GREEN, 2);
    
    // Initialize Storage (NVS + LittleFS)
    Serial.println("Initializing Internal Storage...");
    storage = new StorageModule();
    if (storage->begin()) {
        Serial.println("Storage initialized successfully");
        display->drawText(10, 150, "Storage: OK", ILI9341_GREEN, 2);
        
        // Load saved configuration
        uint8_t savedHour, savedMin;
        bool savedEnabled;
        float savedFreq;
        storage->loadConfig(savedHour, savedMin, savedEnabled, savedFreq);
        alarmState.hour = savedHour;
        alarmState.minute = savedMin;
        alarmState.enabled = savedEnabled;
        fmRadio->setFrequency(savedFreq);
    } else {
        Serial.println("Storage initialization failed!");
        display->drawText(10, 150, "Storage: FAIL", ILI9341_RED, 2);
    }
    
    // Initialize alarm state
    if (alarmState.hour == 0 && alarmState.minute == 0) {
        alarmState.hour = 7;
        alarmState.minute = 0;
    }
    alarmState.enabled = false;
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
    
    Serial.println("Setup complete!");
}

// ===== MAIN LOOP =====
void loop() {
    static unsigned long lastUpdate = 0;
    static unsigned long lastWiFiCheck = 0;
    unsigned long now = millis();
    
    // Check WiFi connection periodically (every 30 seconds)
    if (now - lastWiFiCheck >= 30000) {
        if (!timeModule->isWiFiConnected()) {
            Serial.println("WiFi disconnected, attempting reconnect...");
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

// ===== BUTTON HANDLING =====
void handleButtons() {
    unsigned long now = millis();
    
    // Debounce check
    if (now - ui.lastButtonPress < DEBOUNCE_DELAY) {
        return;
    }
    
    // Read buttons (active LOW)
    bool btnUp = !digitalRead(BTN_UP);
    bool btnDown = !digitalRead(BTN_DOWN);
    bool btnSelect = !digitalRead(BTN_SELECT);
    bool btnSnooze = !digitalRead(BTN_SNOOZE);
    
    if (!btnUp && !btnDown && !btnSelect && !btnSnooze) {
        return;  // No button pressed
    }
    
    ui.lastButtonPress = now;
    buzzer->playBeep();
    
    // Handle snooze button (works in any menu when alarm is triggered)
    if (btnSnooze && alarmState.triggered) {
        snoozeAlarm();
        ui.needsRedraw = true;
        return;
    }
    
    // Menu-specific button handling
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

// ===== MAIN MENU =====
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
        if (ui.currentMenu != MENU_MAIN) {
            display->resetCache(); // Force full redraw when entering menu
        }
        ui.needsRedraw = true;
    }
}

// ===== SET TIME MENU =====
void handleSetTimeMenu(bool up, bool down, bool select) {
    static uint8_t editHour = 12, editMin = 0;
    
    if (ui.selectedItem == 0) {
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
            timeModule->setTime(editHour, editMin, 0);
            ui.currentMenu = MENU_MAIN;
            ui.selectedItem = 0;
            display->clear();
            display->resetCache(); // Force full redraw including clock face
        }
        ui.needsRedraw = true;
    }
}

// ===== SET ALARM MENU =====
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
            display->clear();
            display->resetCache(); // Force full redraw including clock face
        }
        ui.needsRedraw = true;
    }
}

// ===== FM RADIO MENU =====
void handleFMRadioMenu(bool up, bool down, bool select) {
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
            display->clear();
            display->resetCache(); // Force full redraw including clock face
        }
        ui.needsRedraw = true;
    }
}

// ===== SETTINGS MENU =====
void handleSettingsMenu(bool up, bool down, bool select) {
    if (select) {
        ui.currentMenu = MENU_MAIN;
        ui.selectedItem = 0;
        display->clear();
        display->resetCache(); // Force full redraw including clock face
        ui.needsRedraw = true;
    }
}

// ===== DISPLAY UPDATE =====
void updateDisplay() {
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
    // Use smart update functions - only redraws what changed
    display->updateTime(timeModule->getHour(), timeModule->getMinute(), timeModule->getSecond());
    display->updateDate(timeModule->getYear(), timeModule->getMonth(), timeModule->getDay());
    display->updateAlarmStatus(alarmState.enabled, alarmState.hour, alarmState.minute);
    display->updateFMFrequency(fmRadio->getFrequency());
    display->updateWiFiStatus(timeModule->isWiFiConnected());
    
    // Draw menu at bottom (static, only draw if needed)
    static bool menuDrawn = false;
    if (!menuDrawn) {
        display->drawText(10, 220, "UP/DN:Menu SEL:Choose", ILI9341_CYAN, 1);
        menuDrawn = true;
    }
}

void drawSetTimeScreen() {
    Serial.println("41");
    display->drawText(80, 20, "SET TIME", ILI9341_YELLOW, 3);
    
    char timeStr[6];
    Serial.println("42");
    sprintf(timeStr, "%02d:%02d", timeModule->getHour(), timeModule->getMinute());
    
    uint16_t color = (ui.selectedItem == 0) ? ILI9341_GREEN : ILI9341_WHITE;
        Serial.println("43");
    display->drawText(60, 100, timeStr, color, 4);
        Serial.println("44");
    
    display->drawText(10, 180, "UP/DN:Change SEL:Save", ILI9341_CYAN, 1);
    display->drawText(10, 200, "Note: Syncs with NTP", ILI9341_YELLOW, 1);
}

void drawSetAlarmScreen() {
    display->drawText(70, 20, "SET ALARM", ILI9341_YELLOW, 3);
    
    char alarmStr[6];
    sprintf(alarmStr, "%02d:%02d", alarmState.hour, alarmState.minute);
    
    uint16_t color = (ui.selectedItem == 0) ? ILI9341_GREEN : ILI9341_WHITE;
    display->drawText(60, 100, alarmStr, color, 4);
    
    display->drawText(30, 180, "UP/DN:Change SEL:Save", ILI9341_CYAN, 1);
}

void drawFMRadioScreen() {
    display->drawText(80, 20, "FM RADIO", ILI9341_YELLOW, 3);
    
    char freqStr[10];
    sprintf(freqStr, "%.1f MHz", fmRadio->getFrequency());
    display->drawText(70, 100, freqStr, ILI9341_WHITE, 3);
    
    display->drawText(20, 180, "UP/DN:Tune/Seek SEL:Back", ILI9341_CYAN, 1);
}

void drawSettingsScreen() {
    display->drawText(70, 20, "SETTINGS", ILI9341_YELLOW, 3);
    display->drawText(50, 100, "Press SELECT", ILI9341_WHITE, 2);
    display->drawText(70, 130, "to return", ILI9341_WHITE, 2);
}

// ===== ALARM FUNCTIONS =====
void checkAlarm() {
    if (!alarmState.enabled || alarmState.triggered) {
        return;
    }
    
    uint8_t currentHour = timeModule->getHour();
    uint8_t currentMin = timeModule->getMinute();
    uint8_t currentSec = timeModule->getSecond();
    
    // Check if snoozed alarm should trigger
    if (alarmState.snoozed && millis() >= alarmState.snoozeTime) {
        triggerAlarm();
        alarmState.snoozed = false;
        return;
    }
    
    // Check regular alarm time
    if (currentHour == alarmState.hour && currentMin == alarmState.minute && currentSec == 0) {
        triggerAlarm();
    }
}

void triggerAlarm() {
    alarmState.triggered = true;
    buzzer->playAlarm();
    display->clear();
    display->drawText(60, 100, "WAKE UP!", ILI9341_RED, 4);
    display->drawText(40, 150, "Press SNOOZE", ILI9341_WHITE, 2);
}

void snoozeAlarm() {
    alarmState.triggered = false;
    alarmState.snoozed = true;
    alarmState.snoozeTime = millis() + SNOOZE_DURATION;
    buzzer->stopTone();
    
    display->clear();
    display->drawText(70, 100, "SNOOZED", ILI9341_YELLOW, 3);
    display->drawText(60, 140, "5 minutes", ILI9341_WHITE, 2);
    delay(2000);
}

// ===== UTILITY FUNCTIONS =====
void saveConfig() {
    if (storage && storage->isReady()) {
        storage->saveConfig(alarmState.hour, alarmState.minute, alarmState.enabled, fmRadio->getFrequency());
        Serial.println("Configuration saved to NVS");
    }
}

// ===== TEMPERATURE READING (BONUS) =====
float getTemperature() {
    // ESP32 internal temperature sensor (rough estimate)
    // Note: This is not very accurate
    return temperatureRead();
}