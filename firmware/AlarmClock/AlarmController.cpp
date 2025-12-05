#include "AlarmController.h"

AlarmController::AlarmController(AudioModule* aud, DisplayILI9341* disp)
    : audio(aud), display(disp) {
}

void AlarmController::checkAlarm(AlarmState* alarm, TimeModule* time) {
    if (!alarm || !time) return;
    if (!alarm->enabled || alarm->triggered) return;
    
    // Check if snoozed alarm should trigger
    if (alarm->snoozed && millis() >= alarm->snoozeTime) {
        triggerAlarm(alarm);
        alarm->snoozed = false;
        return;
    }
    
    // Check regular alarm time
    uint8_t currentHour = time->getHour();
    uint8_t currentMin = time->getMinute();
    uint8_t currentSec = time->getSecond();
    
    if (currentHour == alarm->hour && 
        currentMin == alarm->minute && 
        currentSec == 0) {
        triggerAlarm(alarm);
    }
}

void AlarmController::triggerAlarm(AlarmState* alarm) {
    if (!alarm) return;
    
    alarm->triggered = true;
    
    Serial.println("ALARM TRIGGERED!");
    
    // Start playing first station if available
    if (audio && audio->getStationCount() > 0) {
        audio->playStation(0);
    }
    
    // Show alarm screen
    if (display) {
        display->clear();
        display->drawText(60, 100, "WAKE UP!", ILI9341_RED, 4);
        display->drawText(40, 150, "Press SNOOZE", ILI9341_WHITE, 2);
    }
}

void AlarmController::snoozeAlarm(AlarmState* alarm) {
    if (!alarm) return;
    
    Serial.println("Alarm snoozed for 5 minutes");
    
    alarm->triggered = false;
    alarm->snoozed = true;
    alarm->snoozeTime = millis() + SNOOZE_DURATION;
    
    // Stop audio (optional - you might want to keep it playing)
    // if (audio) audio->stop();
    
    // Show snooze screen
    if (display) {
        display->clear();
        display->drawText(70, 100, "SNOOZED", ILI9341_YELLOW, 3);
        display->drawText(60, 140, "5 minutes", ILI9341_WHITE, 2);
    }
    
    delay(2000);
}

void AlarmController::stopAlarm(AlarmState* alarm) {
    if (!alarm) return;
    
    Serial.println("Alarm stopped");
    
    alarm->triggered = false;
    alarm->snoozed = false;
    
    // Optionally stop audio
    // if (audio) audio->stop();
}
