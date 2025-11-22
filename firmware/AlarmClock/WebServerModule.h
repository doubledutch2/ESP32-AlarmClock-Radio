u// ==================== WebServerModule.h ====================
#ifndef WEBSERVER_MODULE_H
#define WEBSERVER_MODULE_H

#include <WebServer.h>
#include <ESPmDNS.h>

// Forward declaration
struct RadioStation;

class WebServerModule {
private:
  WebServer* server;
  void handleRoot();
  void handlePlay();
  void handleStations();
  
  // Callback function pointer
  typedef void (*PlayCallback)(const char* name, const char* url);
  PlayCallback playCallback;
  
public:
  WebServerModule();
  ~WebServerModule();
  
  bool begin(const char* mdnsName);
  void handleClient();
  void setPlayCallback(PlayCallback callback);
  
  // Make instance static so it can be used in server callbacks
  static WebServerModule* instance;
};

#endif
