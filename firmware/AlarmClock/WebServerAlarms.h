#ifndef WEBSERVER_ALARMS_H
#define WEBSERVER_ALARMS_H

#include <WebServer.h>
#include "StorageModule.h"
#include "AudioModule.h"
#include "FMRadioModule.h"
#include "AlarmData.h"
#include "CommonTypes.h"
#include "Config.h"

// Forward declaration
class AlarmController;

class WebServerAlarms {
private:
    WebServer* server;
    StorageModule* storage;
    AudioModule* audio;
    FMRadioModule* fmRadio;
    AlarmController* alarmController;  // NEW: Reference to alarm controller
    InternetRadioStation* stationList;
    int stationCount;
    
    void handleAlarms();
    void handleSaveAlarm();
    void handleTestAlarm();
    void handleListMP3();
    
    String getHTMLHeader();
    String getHTMLFooter();
    String getAlarmsHTML();
    String getMP3FilesList();  // NEW: Get list of MP3 files for dropdown

public:
    WebServerAlarms(WebServer* srv, StorageModule* stor, AudioModule* aud, FMRadioModule* fm);
    
    void setupRoutes();
    void setStationList(InternetRadioStation* stations, int count);
    void setAlarmController(AlarmController* ctrl);  // NEW: Set alarm controller reference
};

#endif