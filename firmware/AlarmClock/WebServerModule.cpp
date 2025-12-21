#include "WebServerModule.h"
#include "FeatureFlags.h"
#include "StorageModule.h"
#include "AudioModule.h"
#include "FMRadioModule.h"
#include "AlarmController.h"
#include "WebServerAlarms.h"
#include "DisplayILI9341.h"
#include "WebServerHTML.h"

WebServerModule::WebServerModule() 
    : server(nullptr), playCallback(nullptr), storage(nullptr), 
      timeModule(nullptr), audioModule(nullptr), fmRadioModule(nullptr),
      displayModule(nullptr), stationList(nullptr), stationCount(0), 
      alarmServer(nullptr), alarmController(nullptr) {
    server = new WebServer(80);
}

WebServerModule::~WebServerModule() {
    if (alarmServer) {
        delete alarmServer;
    }
    if (server) {
        delete server;
    }
}

void WebServerModule::begin(const char* mdnsName) {
    // Setup mDNS
    if (MDNS.begin(mdnsName)) {
        Serial.printf("mDNS started: %s.local\n", mdnsName);
        MDNS.addService("http", "tcp", 80);
    } else {
        Serial.println("Error starting mDNS");
    }
    
    // Setup main routes
    server->on("/", [this]() { handleRoot(); });
    server->on("/stations", [this]() { handleStations(); });
    server->on("/add_station", HTTP_POST, [this]() { handleAddStation(); });
    server->on("/delete_station", HTTP_POST, [this]() { handleDeleteStation(); });
    server->on("/settings", [this]() { handleSettings(); });
    server->on("/save_timezone", HTTP_POST, [this]() { handleSaveTimezone(); });
    server->on("/save_features", HTTP_POST, [this]() { handleSaveFeatures(); });
    server->on("/control", [this]() { handleControl(); });
    server->on("/set_volume", HTTP_POST, [this]() { handleSetVolume(); });
    server->on("/set_brightness", HTTP_POST, [this]() { handleSetBrightness(); });
    server->on("/play", HTTP_POST, [this]() { handlePlay(); });
    server->on("/stop", HTTP_POST, [this]() { handleStop(); });
    server->onNotFound([this]() { handleNotFound(); });

    Serial.println("Main routes registered");
    
    // Setup alarm routes if all required modules are available
    if (storage && audioModule) {
        Serial.println("Creating WebServerAlarms...");
        alarmServer = new WebServerAlarms(server, storage, audioModule, fmRadioModule);
        
        if (stationList && stationCount > 0) {
            Serial.printf("Setting station list (%d stations) for alarms\n", stationCount);
            alarmServer->setStationList(stationList, stationCount);
        } else {
            Serial.println("WARNING: No station list available for alarms");
        }
        
        alarmServer->setupRoutes();
        Serial.println("Alarm routes registered");
    } else {
        Serial.println("WARNING: WebServerAlarms not created - missing modules:");
        if (!storage) Serial.println("  - Storage module missing");
        if (!audioModule) Serial.println("  - Audio module missing");
    }
    
    server->begin();
    Serial.println("Web server started on port 80");
    Serial.println("Available routes:");
    Serial.println("  /               - Main page");
    Serial.println("  /control        - Volume/Brightness control");
    Serial.println("  /stations       - Station management");
    Serial.println("  /settings       - Settings & Features");
    if (alarmServer) {
        Serial.println("  /alarms         - Alarm management");
    }
}

void WebServerModule::handleClient() {
    if (server) {
        server->handleClient();
    }
}

void WebServerModule::setPlayCallback(PlayCallback callback) {
    playCallback = callback;
}

void WebServerModule::setStorageModule(StorageModule* stor) {
    storage = stor;
}

void WebServerModule::setTimeModule(TimeModule* time) {
    timeModule = time;
}

void WebServerModule::setAudioModule(AudioModule* aud) {
    audioModule = aud;
}

void WebServerModule::setFMRadioModule(FMRadioModule* fm) {
    fmRadioModule = fm;
}

void WebServerModule::setDisplayModule(DisplayILI9341* disp) {
    displayModule = disp;
}

void WebServerModule::setStationList(InternetRadioStation* stations, int count) {
    stationList = stations;
    stationCount = count;
}

void WebServerModule::setAlarmController(AlarmController* ctrl) {
    alarmController = ctrl;
    Serial.println("WebServerModule: AlarmController reference set");
    
    // If alarmServer already exists, pass it the reference
    if (alarmServer && alarmController) {
        alarmServer->setAlarmController(alarmController);
    }
}

// ===== ROUTE HANDLERS =====

void WebServerModule::handleRoot() {
    server->send(200, "text/html", getMainHTML());
}

void WebServerModule::handleControl() {
    server->send(200, "text/html", getControlHTML());
}

void WebServerModule::handleStations() {
    server->send(200, "text/html", getStationsHTML());
}

void WebServerModule::handleSettings() {
    server->send(200, "text/html", getSettingsHTML());
}

void WebServerModule::handleSetVolume() {
    if (!server->hasArg("volume") || !audioModule || !storage) {
        server->send(400, "text/plain", "Missing parameters");
        return;
    }
    
    int volume = server->arg("volume").toInt();
    audioModule->setVolume(volume);
    storage->saveVolume(volume);
    
    Serial.printf("Volume set to %d via web\n", volume);
    server->send(200, "text/plain", "Volume updated");
}

void WebServerModule::handleSetBrightness() {
    if (!server->hasArg("brightness") || !displayModule || !storage) {
        server->send(400, "text/plain", "Missing parameters");
        return;
    }
    
    int brightness = server->arg("brightness").toInt();
    displayModule->setBrightness(brightness);
    storage->saveBrightness(brightness);
    
    Serial.printf("Brightness set to %d via web\n", brightness);
    server->send(200, "text/plain", "Brightness updated");
}

void WebServerModule::handleSaveFeatures() {
    if (!storage) {
        server->send(400, "text/plain", "Storage not available");
        return;
    }
    
    FeatureFlags flags;
    flags.enableTouchScreen = server->hasArg("touchscreen");
    flags.enableButtons = server->hasArg("buttons");
    flags.enableDraw = server->hasArg("draw");
    flags.enableAudio = server->hasArg("audio");
    flags.enableStereo = server->hasArg("stereo");
    flags.enableLED = server->hasArg("led");
    flags.enableAlarms = server->hasArg("alarms");
    flags.enableWeb = server->hasArg("web");
    flags.enableFMRadio = server->hasArg("fmradio");
    flags.enablePRAM = server->hasArg("pram");
    flags.enableI2CScan = server->hasArg("i2cscan");
    
    if (storage->saveFeatureFlags(flags)) {
        server->send(200, "text/plain", "Feature flags saved. Restart device for changes to take effect.");
    } else {
        server->send(500, "text/plain", "Failed to save feature flags");
    }
}

void WebServerModule::handleAddStation() {
    if (!server->hasArg("name") || !server->hasArg("url") || !storage) {
        server->send(400, "text/plain", "Missing parameters or storage not available");
        return;
    }
    
    String name = server->arg("name");
    String url = server->arg("url");
    
    int count = storage->getInternetStationCount();
    if (count >= MAX_INTERNET_STATIONS) {
        server->send(400, "text/plain", "Maximum stations reached (10)");
        return;
    }
    
    if (storage->saveInternetStation(count, name.c_str(), url.c_str())) {
        storage->setInternetStationCount(count + 1);
        server->send(200, "text/plain", "Station added successfully");
    } else {
        server->send(500, "text/plain", "Failed to save station");
    }
}

void WebServerModule::handleDeleteStation() {
    if (!server->hasArg("index") || !storage) {
        server->send(400, "text/plain", "Missing parameters");
        return;
    }
    
    int index = server->arg("index").toInt();
    int count = storage->getInternetStationCount();
    
    if (index < 0 || index >= count) {
        server->send(400, "text/plain", "Invalid station index");
        return;
    }
    
    // Shift all stations down
    for (int i = index; i < count - 1; i++) {
        String name, url;
        if (storage->loadInternetStation(i + 1, name, url)) {
            storage->saveInternetStation(i, name.c_str(), url.c_str());
        }
    }
    
    // Delete the last station
    storage->deleteInternetStation(count - 1);
    storage->setInternetStationCount(count - 1);
    
    server->send(200, "text/plain", "Station deleted successfully");
}

void WebServerModule::handleSaveTimezone() {
    if (!server->hasArg("gmtOffset") || !server->hasArg("dstOffset") || !storage || !timeModule) {
        server->send(400, "text/plain", "Missing parameters");
        return;
    }
    
    long gmtOffset = server->arg("gmtOffset").toInt() * 3600; // Convert hours to seconds
    int dstOffset = server->arg("dstOffset").toInt() * 3600;  // Convert hours to seconds
    
    if (storage->saveTimezone(gmtOffset, dstOffset)) {
        server->send(200, "text/plain", "Timezone saved. Restart device for changes to take effect.");
    } else {
        server->send(500, "text/plain", "Failed to save timezone");
    }
}

void WebServerModule::handlePlay() {
    if (!server->hasArg("name") || !server->hasArg("url")) {
        server->send(400, "text/plain", "Missing parameters");
        return;
    }
    
    String name = server->arg("name");
    String url = server->arg("url");
    
    Serial.printf("Web request to play: %s - %s\n", name.c_str(), url.c_str());
    
    if (playCallback) {
        playCallback(name.c_str(), url.c_str());
        server->send(200, "text/plain", "Playing: " + name);
    } else {
        server->send(500, "text/plain", "No callback registered");
    }
}

void WebServerModule::handleStop() {
    Serial.println("Web request to stop audio");
    
    if (audioModule) {
        audioModule->stop();
        server->send(200, "text/plain", "Audio stopped");
    } else {
        server->send(500, "text/plain", "Audio module not available");
    }
}

void WebServerModule::handleNotFound() {
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += server->uri();
    message += "\nMethod: ";
    message += (server->method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += server->args();
    message += "\n";
    
    for (uint8_t i = 0; i < server->args(); i++) {
        message += " " + server->argName(i) + ": " + server->arg(i) + "\n";
    }
    
    Serial.println("404 Not Found:");
    Serial.println(message);
    
    server->send(404, "text/plain", message);
}

// ===== HTML GENERATION (delegated to WebServerHTML) =====

String WebServerModule::getControlHTML() {
    return WebServerHTML::getControlHTML(audioModule, displayModule, timeModule);
}

String WebServerModule::getSettingsHTML() {
    return WebServerHTML::getSettingsHTML(storage, timeModule);
}

String WebServerModule::getMainHTML() {
    String html = WebServerHTML::getHTMLHeader();
    
    html += R"html(
        <div class="nav">
            <a href="/" class="active">Play</a>
            <a href="/control">Control</a>
            <a href="/stations">Stations</a>
            <a href="/alarms">Alarms</a>
            <a href="/settings">Settings</a>
        </div>
        <div class="content">
            <div id="alert" class="alert"></div>
            
            <div class="info-box">
                <h3>Quick Play</h3>
                <p>Enter a station name and URL to play immediately, or select from your saved stations below.</p>
            </div>
            
            <div class="form-group">
                <label>Station Name</label>
                <input type="text" id="playName" placeholder="e.g., BBC Radio 5 Live">
            </div>
            
            <div class="form-group">
                <label>Stream URL</label>
                <input type="text" id="playUrl" placeholder="https://...">
            </div>
            
            <button class="btn-primary" onclick="playCustom()">▶ Play Now</button>
            <button class="btn-warning" onclick="stopAudio()">⏸ Pause</button>
            
            <h2 style="margin-top: 40px; margin-bottom: 20px; color: #495057;">Your Saved Stations</h2>
)html";

    // Load and display saved stations
    if (storage) {
        int count = storage->getInternetStationCount();
        
        if (count == 0) {
            html += "<p style='color: #6c757d; text-align: center; padding: 40px;'>No stations saved yet. Add some in the Manage Stations page!</p>";
        } else {
            for (int i = 0; i < count; i++) {
                String name, url;
                if (storage->loadInternetStation(i, name, url)) {
                    html += "<div class='station-card'>";
                    html += "<div class='station-info'>";
                    html += "<h3>" + name + "</h3>";
                    html += "<p>" + url + "</p>";
                    html += "</div>";
                    html += "<button class='btn-success' onclick='playStation(\"" + name + "\", \"" + url + "\")'>▶ Play</button>";
                    html += "</div>";
                }
            }
        }
    }
    
    html += R"html(
        </div>
        <script>
            function playCustom() {
                const name = document.getElementById('playName').value;
                const url = document.getElementById('playUrl').value;
                
                if (!name || !url) {
                    showAlert('Please enter both name and URL', true);
                    return;
                }
                
                playStation(name, url);
            }
            
            function playStation(name, url) {
                fetch('/play', {
                    method: 'POST',
                    headers: {'Content-Type': 'application/x-www-form-urlencoded'},
                    body: 'name=' + encodeURIComponent(name) + '&url=' + encodeURIComponent(url)
                })
                .then(response => response.text())
                .then(data => {
                    showAlert(data);
                })
                .catch(error => {
                    showAlert('Error: ' + error, true);
                });
            }
            
            function stopAudio() {
                fetch('/stop', {
                    method: 'POST'
                })
                .then(response => response.text())
                .then(data => {
                    showAlert(data);
                })
                .catch(error => {
                    showAlert('Error: ' + error, true);
                });
            }
        </script>
)html";
    
    html += WebServerHTML::getHTMLFooter();
    return html;
}

String WebServerModule::getStationsHTML() {
    String html = WebServerHTML::getHTMLHeader();
    
    html += R"html(
        <div class="nav">
            <a href="/">Play</a>
            <a href="/control">Control</a>
            <a href="/stations" class="active">Stations</a>
            <a href="/alarms">Alarms</a>
            <a href="/settings">Settings</a>
        </div>
        <div class="content">
            <div id="alert" class="alert"></div>
            
            <div class="info-box">
                <h3>Station Management</h3>
                <p>Add up to 10 internet radio stations. These will be stored permanently and available for quick playback.</p>
            </div>
            
            <h2 style="margin-bottom: 20px; color: #495057;">Add New Station</h2>
            
            <div class="form-group">
                <label>Station Name</label>
                <input type="text" id="newName" placeholder="e.g., BBC Radio 5 Live">
            </div>
            
            <div class="form-group">
                <label>Stream URL</label>
                <input type="text" id="newUrl" placeholder="https://...">
            </div>
            
            <button class="btn-primary" onclick="addStation()">➕ Add Station</button>
            
            <h2 style="margin-top: 40px; margin-bottom: 20px; color: #495057;">Saved Stations</h2>
            
            <div id="stationsList">
)html";

    // Load and display saved stations with delete buttons
    if (storage) {
        int count = storage->getInternetStationCount();
        
        if (count == 0) {
            html += "<p style='color: #6c757d; text-align: center; padding: 40px;'>No stations saved yet.</p>";
        } else {
            for (int i = 0; i < count; i++) {
                String name, url;
                if (storage->loadInternetStation(i, name, url)) {
                    html += "<div class='station-card' id='station-" + String(i) + "'>";
                    html += "<div class='station-info'>";
                    html += "<h3>" + name + "</h3>";
                    html += "<p>" + url + "</p>";
                    html += "</div>";
                    html += "<div class='station-actions'>";
                    html += "<button class='btn-success' onclick='playFromList(\"" + name + "\", \"" + url + "\")'>▶ Play</button>";
                    html += "<button class='btn-danger' onclick='deleteStation(" + String(i) + ")'>🗑 Delete</button>";
                    html += "</div>";
                    html += "</div>";
                }
            }
        }
    }
    
    html += R"html(
            </div>
        </div>
        <script>
            function addStation() {
                const name = document.getElementById('newName').value;
                const url = document.getElementById('newUrl').value;
                
                if (!name || !url) {
                    showAlert('Please enter both name and URL', true);
                    return;
                }
                
                fetch('/add_station', {
                    method: 'POST',
                    headers: {'Content-Type': 'application/x-www-form-urlencoded'},
                    body: 'name=' + encodeURIComponent(name) + '&url=' + encodeURIComponent(url)
                })
                .then(response => response.text())
                .then(data => {
                    if (data.includes('success')) {
                        showAlert(data);
                        document.getElementById('newName').value = '';
                        document.getElementById('newUrl').value = '';
                        setTimeout(() => location.reload(), 1500);
                    } else {
                        showAlert(data, true);
                    }
                })
                .catch(error => {
                    showAlert('Error: ' + error, true);
                });
            }
            
            function deleteStation(index) {
                if (!confirm('Are you sure you want to delete this station?')) {
                    return;
                }
                
                fetch('/delete_station', {
                    method: 'POST',
                    headers: {'Content-Type': 'application/x-www-form-urlencoded'},
                    body: 'index=' + index
                })
                .then(response => response.text())
                .then(data => {
                    showAlert(data);
                    setTimeout(() => location.reload(), 1000);
                })
                .catch(error => {
                    showAlert('Error: ' + error, true);
                });
            }
            
            function playFromList(name, url) {
                fetch('/play', {
                    method: 'POST',
                    headers: {'Content-Type': 'application/x-www-form-urlencoded'},
                    body: 'name=' + encodeURIComponent(name) + '&url=' + encodeURIComponent(url)
                })
                .then(response => response.text())
                .then(data => {
                    showAlert(data);
                })
                .catch(error => {
                    showAlert('Error: ' + error, true);
                });
            }
        </script>
)html";
    
    html += WebServerHTML::getHTMLFooter();
    return html;
}