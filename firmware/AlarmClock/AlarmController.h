#ifndef ALARM_CONTROLLER_H
#define ALARM_CONTROLLER_H

#include <Arduino.h>
#include "AlarmData.h"
#include "AudioModule.h"
#include "FMRadioModule.h"
#include "DisplayILI9341.h"
#include "TimeModule.h"
#include "StorageModule.h"

#define SNOOZE_DURATION (5 * 60 * 1000)  // 5 minutes

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
    void playAlarmSound(int index);
    void updateLastTriggeredDate(int index, TimeModule* time);

public:
    AlarmController(AudioModule* aud, FMRadioModule* fm, DisplayILI9341* disp, StorageModule* stor);
    
    void begin();
    void checkAlarms(TimeModule* time);
    void snoozeAlarm();
    void stopAlarm();
    
    bool isAlarmTriggered() { return alarmIsTriggered; }
    int getTriggeredAlarmIndex() { return triggeredAlarmIndex; }
};

#endif
