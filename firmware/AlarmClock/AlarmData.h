#ifndef ALARM_DATA_H
#define ALARM_DATA_H

#include <Arduino.h>

// Alarm repeat modes
enum AlarmRepeat {
    ALARM_ONCE = 0,
    ALARM_DAILY = 1,
    ALARM_WEEKDAYS = 2,
    ALARM_WEEKENDS = 3
};

// Alarm sound types
enum AlarmSoundType {
    SOUND_INTERNET_RADIO = 0,
    SOUND_FM_RADIO = 1,
    SOUND_MP3_FILE = 2
};

// Alarm configuration structure
struct AlarmConfig {
    bool enabled;
    uint8_t hour;
    uint8_t minute;
    AlarmRepeat repeatMode;
    AlarmSoundType soundType;
    
    // Sound source identifiers
    int stationIndex;      // For internet radio (index in station list)
    float fmFrequency;     // For FM radio
    String mp3File;        // For MP3 file (filename only, e.g., "alarm1.mp3")
    
    // Last triggered info (to prevent multiple triggers on same day for ONCE mode)
    uint16_t lastYear;
    uint8_t lastMonth;
    uint8_t lastDay;
    
    // Default constructor
    AlarmConfig() {
        enabled = false;
        hour = 7;
        minute = 0;
        repeatMode = ALARM_DAILY;
        soundType = SOUND_INTERNET_RADIO;
        stationIndex = 0;
        fmFrequency = 98.0;
        mp3File = "";
        lastYear = 0;
        lastMonth = 0;
        lastDay = 0;
    }
};

#define MAX_ALARMS 3

#endif
