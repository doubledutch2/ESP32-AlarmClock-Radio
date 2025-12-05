#ifndef ALARM_CONTROLLER_H
#define ALARM_CONTROLLER_H

#include <Arduino.h>
#include "MenuSystem.h"  // For AlarmState struct
#include "BuzzerModule.h"
#include "AudioModule.h"
#include "DisplayILI9341.h"
#include "TimeModule.h"

#define SNOOZE_DURATION (5 * 60 * 1000)  // 5 minutes

class AlarmController {
private:
    AudioModule* audio;
    DisplayILI9341* display;

public:
    AlarmController(AudioModule* aud, DisplayILI9341* disp);
    
    void checkAlarm(AlarmState* alarm, TimeModule* time);
    void triggerAlarm(AlarmState* alarm);
    void snoozeAlarm(AlarmState* alarm);
    void stopAlarm(AlarmState* alarm);
};

#endif
