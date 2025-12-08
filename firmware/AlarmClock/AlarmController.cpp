#include "AlarmController.h"

AlarmController::AlarmController(AudioModule* aud, FMRadioModule* fm, DisplayILI9341* disp, StorageModule* stor)
    : audio(aud), fmRadio(fm), display(disp), storage(stor),
      triggeredAlarmIndex(-1), alarmIsTriggered(false), alarmIsSnoozed(false), snoozeTime(0) {
}

void AlarmController::begin() {
    // Load all alarms from storage
    if (storage) {
        Serial.println("=== Loading Alarms from Storage ===");
        for (int i = 0; i < MAX_ALARMS; i++) {
            storage->loadAlarm(i, alarms[i]);
            Serial.printf("Alarm %d: %02d:%02d %s, Repeat: %d, Sound: %d\n", 
                         i, alarms[i].hour, alarms[i].minute,
                         alarms[i].enabled ? "ENABLED" : "disabled",
                         alarms[i].repeatMode, alarms[i].soundType);
        }
        Serial.println("===================================");
    } else {
        Serial.println("ERROR: Storage module not available for loading alarms");
    }
}

void AlarmController::checkAlarms(TimeModule* time) {
    if (!time) {
        Serial.println("ERROR: TimeModule is NULL in checkAlarms");
        return;
    }
    
    // Check if snoozed alarm should trigger
    if (alarmIsSnoozed && millis() >= snoozeTime) {
        Serial.println("Snooze time expired, re-triggering alarm");
        alarmIsTriggered = true;
        alarmIsSnoozed = false;
        playAlarmSound(triggeredAlarmIndex);
        
        if (display) {
            display->clear();
            display->drawText(60, 100, "WAKE UP!", ILI9341_RED, 4);
            display->drawText(40, 150, "Press SNOOZE", ILI9341_WHITE, 2);
        }
        return;
    }
    
    // Don't check for new alarms if one is already triggered
    if (alarmIsTriggered) return;
    
    // Get current time
    uint8_t currentHour = time->getHour();
    uint8_t currentMin = time->getMinute();
    uint8_t currentSec = time->getSecond();
    
    // Debug: Print current time every minute at 0 seconds
    static uint8_t lastDebugMin = 255;
    if (currentSec == 0 && currentMin != lastDebugMin) {
        lastDebugMin = currentMin;
        Serial.printf("Current time: %02d:%02d:%02d, Checking alarms...\n", 
                     currentHour, currentMin, currentSec);
        
        // Show status of all alarms
        for (int i = 0; i < MAX_ALARMS; i++) {
            if (alarms[i].enabled) {
                Serial.printf("  Alarm %d: %02d:%02d (%s)\n", 
                             i, alarms[i].hour, alarms[i].minute,
                             alarms[i].enabled ? "ON" : "OFF");
            }
        }
    }
    
    // Check each alarm
    for (int i = 0; i < MAX_ALARMS; i++) {
        if (shouldAlarmTrigger(i, time)) {
            Serial.printf(">>> ALARM %d TRIGGERED! <<<\n", i);
            
            triggeredAlarmIndex = i;
            alarmIsTriggered = true;
            
            // Play the alarm sound
            playAlarmSound(i);
            
            // Update last triggered date
            updateLastTriggeredDate(i, time);
            
            // If it's "once" mode, disable the alarm
            if (alarms[i].repeatMode == ALARM_ONCE) {
                alarms[i].enabled = false;
                if (storage) {
                    storage->saveAlarm(i, alarms[i]);
                }
                Serial.printf("Alarm %d disabled (once mode)\n", i);
            }
            
            // Show alarm screen
            if (display) {
                display->clear();
                display->drawText(60, 100, "WAKE UP!", ILI9341_RED, 4);
                display->drawText(20, 150, ("Alarm " + String(i + 1)).c_str(), ILI9341_WHITE, 3);
                display->drawText(40, 190, "Press SNOOZE", ILI9341_WHITE, 2);
            }
            
            // Only trigger one alarm at a time
            break;
        }
    }
}

bool AlarmController::shouldAlarmTrigger(int index, TimeModule* time) {
    if (index < 0 || index >= MAX_ALARMS) return false;
    
    AlarmConfig& alarm = alarms[index];
    
    // Check if alarm is enabled
    if (!alarm.enabled) {
        return false;
    }
    
    // Get current time
    uint8_t currentHour = time->getHour();
    uint8_t currentMin = time->getMinute();
    uint8_t currentSec = time->getSecond();
    uint8_t dayOfWeek = time->getDayOfWeek();
    
    // Check if it's the right time (trigger only at 0 seconds)
    if (currentHour != alarm.hour || currentMin != alarm.minute || currentSec != 0) {
        return false;
    }
    
    Serial.printf("Time match for Alarm %d! Checking day of week...\n", index);
    
    // Check if it's the right day of week
    if (!isCorrectDayOfWeek(alarm.repeatMode, dayOfWeek)) {
        Serial.printf("  Day of week mismatch (current: %d, mode: %d)\n", dayOfWeek, alarm.repeatMode);
        return false;
    }
    
    Serial.printf("  Day of week OK. Checking if already triggered today...\n");
    
    // Check if already triggered today (prevents multiple triggers)
    if (hasAlreadyTriggeredToday(index, time)) {
        Serial.printf("  Already triggered today\n");
        return false;
    }
    
    Serial.printf("  All checks passed - TRIGGER!\n");
    return true;
}

bool AlarmController::isCorrectDayOfWeek(AlarmRepeat mode, uint8_t dayOfWeek) {
    switch (mode) {
        case ALARM_ONCE:
            return true;  // Can trigger any day (but only once)
            
        case ALARM_DAILY:
            return true;  // Every day
            
        case ALARM_WEEKDAYS:
            // Monday = 1, Friday = 5
            return (dayOfWeek >= 1 && dayOfWeek <= 5);
            
        case ALARM_WEEKENDS:
            // Saturday = 6, Sunday = 0
            return (dayOfWeek == 0 || dayOfWeek == 6);
            
        default:
            return false;
    }
}

bool AlarmController::hasAlreadyTriggeredToday(int index, TimeModule* time) {
    if (index < 0 || index >= MAX_ALARMS) return false;
    
    AlarmConfig& alarm = alarms[index];
    
    uint16_t currentYear = time->getYear();
    uint8_t currentMonth = time->getMonth();
    uint8_t currentDay = time->getDay();
    
    // Check if last triggered date matches today
    if (alarm.lastYear == currentYear && 
        alarm.lastMonth == currentMonth && 
        alarm.lastDay == currentDay) {
        return true;
    }
    
    return false;
}

void AlarmController::updateLastTriggeredDate(int index, TimeModule* time) {
    if (index < 0 || index >= MAX_ALARMS || !time || !storage) return;
    
    alarms[index].lastYear = time->getYear();
    alarms[index].lastMonth = time->getMonth();
    alarms[index].lastDay = time->getDay();
    
    storage->saveAlarm(index, alarms[index]);
    
    Serial.printf("Updated last triggered date for Alarm %d: %04d-%02d-%02d\n",
                  index, alarms[index].lastYear, alarms[index].lastMonth, alarms[index].lastDay);
}

void AlarmController::playAlarmSound(int index) {
    if (index < 0 || index >= MAX_ALARMS) return;
    
    AlarmConfig& alarm = alarms[index];
    
    Serial.printf("Playing alarm sound - Type: %d\n", alarm.soundType);
    
    switch (alarm.soundType) {
        case SOUND_INTERNET_RADIO:
            if (audio) {
                Serial.printf("Playing Internet Radio - Station Index: %d\n", alarm.stationIndex);
                audio->playStation(alarm.stationIndex);
            } else {
                Serial.println("ERROR: Audio module not available");
            }
            break;
            
        case SOUND_FM_RADIO:
            if (fmRadio && fmRadio->isReady()) {
                Serial.printf("Tuning FM Radio to %.1f MHz\n", alarm.fmFrequency);
                fmRadio->setFrequency(alarm.fmFrequency);
            } else {
                Serial.println("FM Radio not available");
            }
            break;
            
        case SOUND_MP3_FILE:
            if (audio && alarm.mp3File.length() > 0) {
                Serial.printf("Playing MP3 file: %s (looped)\n", alarm.mp3File.c_str());
                if (audio->playMP3File(alarm.mp3File.c_str(), true)) {
                    Serial.println("MP3 playback started successfully");
                } else {
                    Serial.println("ERROR: Failed to play MP3 file");
                }
            } else {
                Serial.println("MP3 file not specified or audio not available");
            }
            break;
            
        default:
            Serial.printf("ERROR: Unknown sound type: %d\n", alarm.soundType);
    }
}

void AlarmController::snoozeAlarm() {
    if (!alarmIsTriggered) return;
    
    Serial.println("Alarm snoozed for 5 minutes");
    
    alarmIsTriggered = false;
    alarmIsSnoozed = true;
    snoozeTime = millis() + SNOOZE_DURATION;
    
    // Stop audio
    if (audio) {
        audio->stop();
    }
    
    // Show snooze screen
    if (display) {
        display->clear();
        display->drawText(70, 100, "SNOOZED", ILI9341_YELLOW, 3);
        display->drawText(60, 140, "5 minutes", ILI9341_WHITE, 2);
    }
    
    delay(2000);
}

void AlarmController::stopAlarm() {
    if (!alarmIsTriggered && !alarmIsSnoozed) return;
    
    Serial.println("Alarm stopped");
    
    alarmIsTriggered = false;
    alarmIsSnoozed = false;
    triggeredAlarmIndex = -1;
    
    // Stop audio
    if (audio) {
        audio->stop();
    }
}