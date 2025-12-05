#include "MenuSystem.h"

MenuSystem::MenuSystem(DisplayILI9341* disp, TimeModule* time, FMRadioModule* fm, 
                       AudioModule* aud , StorageModule* stor)
    : display(disp), timeModule(time), fmRadio(fm), 
      audio(aud),  storage(stor),
      alarmState(nullptr), uiState(nullptr),
      stationList(nullptr), stationCount(0), wifiConnected(false) {
}

void MenuSystem::setAlarmState(AlarmState* alarm) {
    alarmState = alarm;
}

void MenuSystem::setUIState(UIState* ui) {
    uiState = ui;
}

void MenuSystem::setStationList(InternetRadioStation* stations, int count) {
    stationList = stations;
    stationCount = count;
    if (audio) {
        audio->setStationList(stations, count);
    }
}

void MenuSystem::setWiFiStatus(bool connected) {
    wifiConnected = connected;
}

void MenuSystem::handleButtons(bool up, bool down, bool select, bool snooze) {
    if (!uiState) return;
    
    // Handle snooze in any menu when alarm is triggered
    if (snooze && alarmState && alarmState->triggered) {
        alarmState->triggered = false;
        alarmState->snoozed = true;
        alarmState->snoozeTime = millis() + (5 * 60 * 1000);
        if (display) {
            display->clear();
            display->drawText(70, 100, "SNOOZED", ILI9341_YELLOW, 3);
            display->drawText(60, 140, "5 minutes", ILI9341_WHITE, 2);
        }
        delay(2000);
        uiState->needsRedraw = true;
        return;
    }
    
    // Route to appropriate menu handler
    switch (uiState->currentMenu) {
        case MENU_MAIN:
            handleMainMenu(up, down, select);
            break;
        case MENU_SET_TIME:
            handleSetTimeMenu(up, down, select);
            break;
        case MENU_SET_ALARM:
            handleSetAlarmMenu(up, down, select);
            break;
        case MENU_FM_RADIO:
            handleFMRadioMenu(up, down, select);
            break;
        case MENU_STATIONS:
            handleStationsMenu(up, down, select);
            break;
        case MENU_SETTINGS:
            handleSettingsMenu(up, down, select);
            break;
    }
}

void MenuSystem::updateDisplay() {
    if (!display || !uiState) return;
    
    switch (uiState->currentMenu) {
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
        case MENU_STATIONS:
            display->clear();
            drawStationsScreen();
            break;
        case MENU_SETTINGS:
            display->clear();
            drawSettingsScreen();
            break;
    }
}

void MenuSystem::saveConfig() {
    if (!storage || !storage->isReady() || !alarmState) return;
    
    float fmFreq = fmRadio ? fmRadio->getFrequency() : 98.0;
    storage->saveConfig(alarmState->hour, alarmState->minute, 
                       alarmState->enabled, fmFreq);
    Serial.println("Configuration saved");
}

// ===== MENU HANDLERS =====

void MenuSystem::handleMainMenu(bool up, bool down, bool select) {
    if (!uiState) return;
    
    if (up) {
        uiState->selectedItem = (uiState->selectedItem - 1 + 5) % 5;
        uiState->needsRedraw = true;
    } else if (down) {
        uiState->selectedItem = (uiState->selectedItem + 1) % 5;
        uiState->needsRedraw = true;
    } else if (select) {
        switch (uiState->selectedItem) {
            case 0:
                uiState->currentMenu = MENU_SET_TIME;
                uiState->selectedItem = 0;
                break;
            case 1:
                uiState->currentMenu = MENU_SET_ALARM;
                uiState->selectedItem = 0;
                break;
            case 2:
                uiState->currentMenu = MENU_FM_RADIO;
                uiState->selectedItem = 0;
                break;
            case 3:
                uiState->currentMenu = MENU_STATIONS;
                uiState->selectedItem = 0;
                break;
            case 4:
                if (alarmState) {
                    alarmState->enabled = !alarmState->enabled;
                    saveConfig();
                }
                break;
        }
        if (uiState->currentMenu != MENU_MAIN && display) {
            display->resetCache();
        }
        uiState->needsRedraw = true;
    }
}

void MenuSystem::handleSetTimeMenu(bool up, bool down, bool select) {
    if (!uiState || !timeModule) return;
    
    static uint8_t editHour = 12, editMin = 0;
    
    if (uiState->selectedItem == 0) {
        editHour = timeModule->getHour();
        editMin = timeModule->getMinute();
    }
    
    if (up) {
        if (uiState->selectedItem == 0) {
            editHour = (editHour + 1) % 24;
        } else if (uiState->selectedItem == 1) {
            editMin = (editMin + 1) % 60;
        }
        uiState->needsRedraw = true;
    } else if (down) {
        if (uiState->selectedItem == 0) {
            editHour = (editHour == 0) ? 23 : editHour - 1;
        } else if (uiState->selectedItem == 1) {
            editMin = (editMin == 0) ? 59 : editMin - 1;
        }
        uiState->needsRedraw = true;
    } else if (select) {
        if (uiState->selectedItem < 1) {
            uiState->selectedItem++;
        } else {
            timeModule->setTime(editHour, editMin, 0);
            uiState->currentMenu = MENU_MAIN;
            uiState->selectedItem = 0;
            if (display) {
                display->clear();
                display->drawClockFace();
                display->resetCache();
            }
        }
        uiState->needsRedraw = true;
    }
}

void MenuSystem::handleSetAlarmMenu(bool up, bool down, bool select) {
    if (!uiState || !alarmState) return;
    
    if (up) {
        if (uiState->selectedItem == 0) {
            alarmState->hour = (alarmState->hour + 1) % 24;
        } else if (uiState->selectedItem == 1) {
            alarmState->minute = (alarmState->minute + 1) % 60;
        }
        uiState->needsRedraw = true;
    } else if (down) {
        if (uiState->selectedItem == 0) {
            alarmState->hour = (alarmState->hour == 0) ? 23 : alarmState->hour - 1;
        } else if (uiState->selectedItem == 1) {
            alarmState->minute = (alarmState->minute == 0) ? 59 : alarmState->minute - 1;
        }
        uiState->needsRedraw = true;
    } else if (select) {
        if (uiState->selectedItem < 1) {
            uiState->selectedItem++;
        } else {
            saveConfig();
            uiState->currentMenu = MENU_MAIN;
            uiState->selectedItem = 0;
            if (display) {
                display->clear();
                display->drawClockFace();
                display->resetCache();
            }
        }
        uiState->needsRedraw = true;
    }
}

void MenuSystem::handleFMRadioMenu(bool up, bool down, bool select) {
    if (!uiState || !fmRadio) return;
    
    if (up) {
        float freq = fmRadio->getFrequency();
        freq += 0.1;
        if (freq > 108.0) freq = 87.0;
        fmRadio->setFrequency(freq);
        uiState->needsRedraw = true;
    } else if (down) {
        float freq = fmRadio->getFrequency();
        freq -= 0.1;
        if (freq < 87.0) freq = 108.0;
        fmRadio->setFrequency(freq);
        uiState->needsRedraw = true;
    } else if (select) {
        saveConfig();
        uiState->currentMenu = MENU_MAIN;
        uiState->selectedItem = 0;
        if (display) {
            display->clear();
            display->drawClockFace();
            display->resetCache();
        }
        uiState->needsRedraw = true;
    }
}

void MenuSystem::handleStationsMenu(bool up, bool down, bool select) {
    if (!uiState) return;
    
    if (up) {
        uiState->selectedItem = (uiState->selectedItem - 1 + stationCount) % stationCount;
        uiState->needsRedraw = true;
    } else if (down) {
        uiState->selectedItem = (uiState->selectedItem + 1) % stationCount;
        uiState->needsRedraw = true;
    } else if (select) {
        if (audio && uiState->selectedItem < stationCount) {
            audio->playStation(uiState->selectedItem);
        }
        uiState->currentMenu = MENU_MAIN;
        uiState->selectedItem = 0;
        if (display) {
            display->clear();
            display->drawClockFace();
            display->resetCache();
        }
        uiState->needsRedraw = true;
    }
}

void MenuSystem::handleSettingsMenu(bool up, bool down, bool select) {
    if (!uiState) return;
    
    if (select) {
        uiState->currentMenu = MENU_MAIN;
        uiState->selectedItem = 0;
        if (display) {
            display->clear();
            display->drawClockFace();
            display->resetCache();
        }
        uiState->needsRedraw = true;
    }
}

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
