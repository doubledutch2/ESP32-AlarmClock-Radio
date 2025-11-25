#ifndef WEBSERVER_MODULE_H
#define WEBSERVER_MODULE_H

#include <WebServer.h>
#include <ESPmDNS.h>

// Callback type for playing custom stations
typedef void (*PlayCallback)(const char* name, const char* url);

class WebServerModule {
private:
    WebServer* server;
    PlayCallback playCallback;
    
    void handleRoot();
    void handlePlay();
    void handleNotFound();
    String getHTML();

public:
    WebServerModule();
    ~WebServerModule();
    
    void begin(const char* mdnsName = "alarmclock");
    void handleClient();
    void setPlayCallback(PlayCallback callback);
};

#endif