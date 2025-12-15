#ifndef WEBSERVER_MODULE_H
#define WEBSERVER_MODULE_H

#include <WebServer.h>
#include <ESPmDNS.h>
#include "StorageModule.h"
#include "TimeModule.h"
#include "CommonTypes.h"

// Forward declarations
class AudioModule;
class FMRadioModule;
class WebServerAlarms;
class AlarmController;

// Callback type for playing custom stations
typedef void (*PlayCallback)(const char* name, const char* url);

class WebServerModule {
private:
    WebServer* server;
    PlayCallback playCallback;
    StorageModule* storage;
    TimeModule* timeModule;
    AudioModule* audioModule;
    FMRadioModule* fmRadioModule;
    InternetRadioStation* stationList;
    int stationCount;
    WebServerAlarms* alarmServer;
    AlarmController* alarmController;
    
    void handleRoot();
    void handleStations();
    void handleAddStation();
    void handleDeleteStation();
    void handleSettings();
    void handleSaveTimezone();
    void handlePlay();
    void handleStop();          // NEW: Stop/pause handler
    void handleNotFound();
    
    String getHTMLHeader();
    String getHTMLFooter();
    String getMainHTML();
    String getStationsHTML();
    String getSettingsHTML();

public:
    WebServerModule();
    ~WebServerModule();
    
    void begin(const char* mdnsName = "alarmclock");
    void handleClient();
    void setPlayCallback(PlayCallback callback);
    void setStorageModule(StorageModule* stor);
    void setTimeModule(TimeModule* time);
    void setAudioModule(AudioModule* aud);      
    void setFMRadioModule(FMRadioModule* fm);   
    void setStationList(InternetRadioStation* stations, int count);  
    void setAlarmController(AlarmController* ctrl);
};

#endif