#ifndef ALARM_CONTROLLER_H
#define ALARM_CONTROLLER_H

#include <Arduino.h>
#include "AudioModule.h"
#include "FMRadioModule.h"
#include "DisplayILI9341.h"
#include "StorageModule.h"
#include "TimeModule.h"
#include "AlarmData.h"

#define MAX_ALARMS 3
#define SNOOZE_DURATION (5 * 60 * 1000)  // 5 minutes in milliseconds

class AlarmController {
private:
    AudioModule* audio;
    FMRadioModule* fmRadio;
    DisplayILI9341* display;
    StorageModule* storage;
    
    AlarmConfig alarms[MAX_ALARMS];
    
    int triggeredAlarmIndex;
    bool alarmIsTriggered;
    bool alarmIsSnoozed;
    unsigned long snoozeTime;
    
    bool shouldAlarmTrigger(int index, TimeModule* time);
    bool isCorrectDayOfWeek(AlarmRepeat mode, uint8_t dayOfWeek);
    bool hasAlreadyTriggeredToday(int index, TimeModule* time);
    void updateLastTriggeredDate(int index, TimeModule* time);
    void playAlarmSound(int index);

public:
    AlarmController(AudioModule* aud, FMRadioModule* fm, DisplayILI9341* disp, StorageModule* stor);
    
    void begin();
    void reloadAlarms();  // NEW: Reload alarms from storage
    void checkAlarms(TimeModule* time);
    void snoozeAlarm();
    void stopAlarm();
    
    bool isAlarmTriggered() { return alarmIsTriggered; }
    bool isAlarmSnoozed() { return alarmIsSnoozed; }
    int getTriggeredAlarmIndex() { return triggeredAlarmIndex; }
    
    // Get alarm configuration (for display/editing)
    AlarmConfig* getAlarm(int index) {
        if (index >= 0 && index < MAX_ALARMS) return &alarms[index];
        return nullptr;
    }
};

#endif