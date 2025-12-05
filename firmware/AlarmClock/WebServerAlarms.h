#ifndef WEBSERVER_ALARMS_H
#define WEBSERVER_ALARMS_H

#include <WebServer.h>
#include "StorageModule.h"
#include "AudioModule.h"
#include "FMRadioModule.h"
#include "AlarmData.h"

class WebServerAlarms {
private:
    WebServer* server;
    StorageModule* storage;
    AudioModule* audio;
    FMRadioModule* fmRadio;
    InternetRadioStation* stationList;
    int stationCount;
    
    void handleAlarms();
    void handleSaveAlarm();
    void handleTestAlarm();
    void handleListMP3();
    
    String getAlarmsHTML();
    String getHTMLHeader();
    String getHTMLFooter();

public:
    WebServerAlarms(WebServer* srv, StorageModule* stor, AudioModule* aud, FMRadioModule* fm);
    
    void setStationList(InternetRadioStation* stations, int count);
    void setupRoutes();
};

#endif
