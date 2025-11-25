// This file contains the screen drawing functions for MenuSystem
// Include this in your project along with MenuSystem.cpp
// Or append it to MenuSystem.cpp

#include "MenuSystem.h"

// ===== SCREEN DRAWING FUNCTIONS =====

void MenuSystem::drawMainScreen() {
    if (!display || !timeModule) return;
    
    // Update time with smart caching (only redraws what changed)
    display->updateTime(timeModule->getHour(), timeModule->getMinute(), timeModule->getSecond());
    display->updateDate(timeModule->getYear(), timeModule->getMonth(), timeModule->getDay());
    
    // Update alarm status
    if (alarmState) {
        display->updateAlarmStatus(alarmState->enabled, alarmState->hour, alarmState->minute);
    }
    
    // Update FM frequency
    if (fmRadio) {
        display->updateFMFrequency(fmRadio->getFrequency());
    }
    
    // Update WiFi status
    display->updateWiFiStatus(wifiConnected);
    
    // Show currently playing station
    if (audio && audio->getIsPlaying()) {
        static String lastStation = "";
        String currentStation = audio->getCurrentStationName();
        if (currentStation != lastStation) {
            display->fillRect(10, 195, 200, 20, ILI9341_BLACK);
            display->drawText(10, 195, currentStation.c_str(), ILI9341_YELLOW, 1);
            lastStation = currentStation;
        }
    }
    
    // Show volume
    if (audio) {
        static int lastVol = -1;
        int vol = audio->getCurrentVolume();
        if (vol != lastVol) {
            display->fillRect(220, 220, 90, 15, ILI9341_BLACK);
            char volStr[20];
            sprintf(volStr, "Vol:%d", vol);
            display->drawText(220, 220, volStr, ILI9341_CYAN, 1);
            lastVol = vol;
        }
    }
    
    // Draw menu hint (only once)
    static bool menuDrawn = false;
    if (!menuDrawn) {
        display->drawText(10, 220, "UP/DN:Menu SEL:Choose", ILI9341_CYAN, 1);
        menuDrawn = true;
    }
}

void MenuSystem::drawSetTimeScreen() {
    if (!display || !timeModule) return;
    
    display->drawText(80, 20, "SET TIME", ILI9341_YELLOW, 3);
    
    char timeStr[6];
    sprintf(timeStr, "%02d:%02d", timeModule->getHour(), timeModule->getMinute());
    
    uint16_t color = (uiState && uiState->selectedItem == 0) ? ILI9341_GREEN : ILI9341_WHITE;
    display->drawText(60, 100, timeStr, color, 4);
    
    display->drawText(10, 180, "UP/DN:Change SEL:Save", ILI9341_CYAN, 1);
    display->drawText(10, 200, "Note: Syncs with NTP", ILI9341_YELLOW, 1);
}

void MenuSystem::drawSetAlarmScreen() {
    if (!display || !alarmState) return;
    
    display->drawText(70, 20, "SET ALARM", ILI9341_YELLOW, 3);
    
    char alarmStr[6];
    sprintf(alarmStr, "%02d:%02d", alarmState->hour, alarmState->minute);
    
    uint16_t color = (uiState && uiState->selectedItem == 0) ? ILI9341_GREEN : ILI9341_WHITE;
    display->drawText(60, 100, alarmStr, color, 4);
    
    display->drawText(30, 180, "UP/DN:Change SEL:Save", ILI9341_CYAN, 1);
}

void MenuSystem::drawFMRadioScreen() {
    if (!display) return;
    
    display->drawText(80, 20, "FM RADIO", ILI9341_YELLOW, 3);
    
    if (fmRadio) {
        char freqStr[12];
        sprintf(freqStr, "%.1f MHz", fmRadio->getFrequency());
        display->drawText(70, 100, freqStr, ILI9341_WHITE, 3);
    } else {
        display->drawText(50, 100, "Not Available", ILI9341_RED, 2);
    }
    
    display->drawText(20, 180, "UP/DN:Tune SEL:Back", ILI9341_CYAN, 1);
}

void MenuSystem::drawStationsScreen() {
    if (!display) return;
    
    display->drawText(60, 20, "STATIONS", ILI9341_YELLOW, 3);
    
    if (stationCount == 0) {
        display->drawText(40, 100, "No Stations", ILI9341_RED, 2);
        display->drawText(10, 200, "SEL: Back", ILI9341_CYAN, 1);
        return;
    }
    
    // Show 5 stations at a time
    int startIdx = uiState ? (uiState->selectedItem / 5) * 5 : 0;
    for (int i = 0; i < 5 && (startIdx + i) < stationCount; i++) {
        uint16_t color = ILI9341_WHITE;
        if (uiState && (startIdx + i) == uiState->selectedItem) {
            color = ILI9341_GREEN;
        }
        
        // Truncate long names
        String name = stationList[startIdx + i].name;
        if (name.length() > 20) {
            name = name.substring(0, 17) + "...";
        }
        
        display->drawText(10, 60 + i * 25, name.c_str(), color, 2);
    }
    
    display->drawText(10, 200, "UP/DN:Select SEL:Play", ILI9341_CYAN, 1);
}

void MenuSystem::drawSettingsScreen() {
    if (!display) return;
    
    display->drawText(70, 20, "SETTINGS", ILI9341_YELLOW, 3);
    
    display->drawText(30, 70, "Brightness:", ILI9341_WHITE, 2);
    char brightStr[10];
    sprintf(brightStr, "%d/5", display->getBrightness() / 50);
    display->drawText(180, 70, brightStr, ILI9341_CYAN, 2);
    
    display->drawText(30, 100, "Web Interface:", ILI9341_WHITE, 2);
    if (wifiConnected) {
        display->drawText(30, 120, "http://alarmclock.local", ILI9341_GREEN, 1);
    } else {
        display->drawText(30, 120, "WiFi Not Connected", ILI9341_RED, 1);
    }
    
    if (audio) {
        display->drawText(30, 150, "Audio Status:", ILI9341_WHITE, 2);
        if (audio->getIsPlaying()) {
            display->drawText(30, 170, "Playing", ILI9341_GREEN, 1);
        } else {
            display->drawText(30, 170, "Stopped", ILI9341_YELLOW, 1);
        }
    }
    
    display->drawText(50, 200, "Press SELECT to return", ILI9341_CYAN, 1);
}
