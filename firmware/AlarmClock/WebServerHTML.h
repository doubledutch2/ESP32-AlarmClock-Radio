#ifndef WEBSERVER_HTML_H
#define WEBSERVER_HTML_H

#include <Arduino.h>

// Forward declarations
class StorageModule;
class TimeModule;
class AudioModule;
class DisplayILI9341;

class WebServerHTML {
public:
    static String getHTMLHeader();
    static String getHTMLFooter();
    static String getControlHTML(AudioModule* audioModule, DisplayILI9341* displayModule, TimeModule* timeModule);
    static String getSettingsHTML(StorageModule* storage, TimeModule* timeModule);
};

#endif
