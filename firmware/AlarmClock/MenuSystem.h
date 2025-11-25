#ifndef MENU_SYSTEM_H
#define MENU_SYSTEM_H

#include <Arduino.h>
#include "DisplayILI9341.h"
#include "TimeModule.h"
#include "FMRadioModule.h"
#include "AudioModule.h"
#include "BuzzerModule.h"
#include "StorageModule.h"
#include "CommonTypes.h"

enum MenuState {
    MENU_MAIN,
    MENU_SET_TIME,
    MENU_SET_ALARM,
    MENU_FM_RADIO,
    MENU_STATIONS,
    MENU_SETTINGS
};

struct AlarmState {
    uint8_t hour;
    uint8_t minute;
    bool enabled;
    bool triggered;
    bool snoozed;
    unsigned long snoozeTime;
};

struct UIState {
    MenuState currentMenu;
    int selectedItem;
    bool needsRedraw;
    unsigned long lastButtonPress;
};

class MenuSystem {
private:
    DisplayILI9341* display;
    TimeModule* timeModule;
    FMRadioModule* fmRadio;
    AudioModule* audio;
    BuzzerModule* buzzer;
    StorageModule* storage;
    
    AlarmState* alarmState;
    UIState* uiState;
    
    InternetRadioStation* stationList;
    int stationCount;
    bool wifiConnected;

public:
    MenuSystem(DisplayILI9341* disp, TimeModule* time, FMRadioModule* fm, 
               AudioModule* aud, BuzzerModule* buzz, StorageModule* stor);
    
    void setAlarmState(AlarmState* alarm);
    void setUIState(UIState* ui);
    void setStationList(InternetRadioStation* stations, int count);
    void setWiFiStatus(bool connected);
    
    void handleButtons(bool up, bool down, bool select, bool snooze);
    void updateDisplay();
    void saveConfig();
    
    // Individual screen handlers
    void handleMainMenu(bool up, bool down, bool select);
    void handleSetTimeMenu(bool up, bool down, bool select);
    void handleSetAlarmMenu(bool up, bool down, bool select);
    void handleFMRadioMenu(bool up, bool down, bool select);
    void handleStationsMenu(bool up, bool down, bool select);
    void handleSettingsMenu(bool up, bool down, bool select);
    
    // Individual screen drawers
    void drawMainScreen();
    void drawSetTimeScreen();
    void drawSetAlarmScreen();
    void drawFMRadioScreen();
    void drawStationsScreen();
    void drawSettingsScreen();
};

#endif
