#include "WebServerModule.h"
#include "AudioModule.h"
#include "FMRadioModule.h"
#include "WebServerAlarms.h"

WebServerModule::WebServerModule() 
    : server(nullptr), playCallback(nullptr), storage(nullptr), 
      timeModule(nullptr), audioModule(nullptr), fmRadioModule(nullptr),
      stationList(nullptr), stationCount(0), alarmServer(nullptr) {
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
    
    // Setup routes
    server->on("/", [this]() { handleRoot(); });
    server->on("/stations", [this]() { handleStations(); });
    server->on("/add_station", HTTP_POST, [this]() { handleAddStation(); });
    server->on("/delete_station", HTTP_POST, [this]() { handleDeleteStation(); });
    server->on("/settings", [this]() { handleSettings(); });
    server->on("/save_timezone", HTTP_POST, [this]() { handleSaveTimezone(); });
    server->on("/play", HTTP_POST, [this]() { handlePlay(); });
    server->onNotFound([this]() { handleNotFound(); });

    if (storage && audioModule && fmRadioModule) {
        alarmServer = new WebServerAlarms(server, storage, audioModule, fmRadioModule);
        alarmServer->setStationList(stationList, stationCount);
        alarmServer->setupRoutes();
    }
    
    server->begin();
    Serial.println("Web server started on port 80");
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

void WebServerModule::setStationList(InternetRadioStation* stations, int count) {
    stationList = stations;
    stationCount = count;
}

void WebServerModule::handleRoot() {
    server->send(200, "text/html", getMainHTML());
}

void WebServerModule::handleStations() {
    server->send(200, "text/html", getStationsHTML());
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

void WebServerModule::handleSettings() {
    server->send(200, "text/html", getSettingsHTML());
}

void WebServerModule::handleSaveTimezone() {
    if (!server->hasArg("gmtOffset") || !server->hasArg("dstOffset") || !storage || !timeModule) {
        server->send(400, "text/plain", "Missing parameters");
        return;
    }
    
    long gmtOffset = server->arg("gmtOffset").toInt() * 3600; // Convert hours to seconds
    int dstOffset = server->arg("dstOffset").toInt() * 3600;  // Convert hours to seconds
    
    if (storage->saveTimezone(gmtOffset, dstOffset)) {
        // Update the time module with new settings
        // Note: This requires a restart to take effect properly
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

void WebServerModule::handleNotFound() {
    server->send(404, "text/plain", "Not Found");
}

String WebServerModule::getHTMLHeader() {
    return R"html(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Alarm Clock Radio</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Arial, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            padding: 20px;
        }
        .container {
            max-width: 800px;
            margin: 0 auto;
            background: white;
            border-radius: 15px;
            box-shadow: 0 10px 40px rgba(0,0,0,0.2);
            overflow: hidden;
        }
        .header {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            padding: 30px;
            text-align: center;
        }
        .header h1 {
            font-size: 2em;
            margin-bottom: 10px;
        }
        .nav {
            display: flex;
            background: #f8f9fa;
            border-bottom: 2px solid #e9ecef;
        }
        .nav a {
            flex: 1;
            padding: 15px;
            text-align: center;
            text-decoration: none;
            color: #495057;
            font-weight: 600;
            transition: all 0.3s;
            border-bottom: 3px solid transparent;
        }
        .nav a:hover {
            background: #e9ecef;
            color: #667eea;
        }
        .nav a.active {
            color: #667eea;
            border-bottom-color: #667eea;
        }
        .content {
            padding: 30px;
        }
        .form-group {
            margin-bottom: 20px;
        }
        .form-group label {
            display: block;
            margin-bottom: 8px;
            color: #495057;
            font-weight: 600;
        }
        .form-group input, .form-group select {
            width: 100%;
            padding: 12px;
            border: 2px solid #e9ecef;
            border-radius: 8px;
            font-size: 16px;
            transition: border-color 0.3s;
        }
        .form-group input:focus, .form-group select:focus {
            outline: none;
            border-color: #667eea;
        }
        button {
            padding: 12px 24px;
            border: none;
            border-radius: 8px;
            font-size: 16px;
            font-weight: 600;
            cursor: pointer;
            transition: all 0.3s;
            margin: 5px;
        }
        .btn-primary {
            background: #667eea;
            color: white;
        }
        .btn-primary:hover {
            background: #5568d3;
            transform: translateY(-2px);
            box-shadow: 0 5px 15px rgba(102, 126, 234, 0.4);
        }
        .btn-success {
            background: #28a745;
            color: white;
        }
        .btn-success:hover {
            background: #218838;
        }
        .btn-danger {
            background: #dc3545;
            color: white;
        }
        .btn-danger:hover {
            background: #c82333;
        }
        .station-card {
            background: #f8f9fa;
            padding: 20px;
            border-radius: 10px;
            margin-bottom: 15px;
            display: flex;
            justify-content: space-between;
            align-items: center;
            transition: all 0.3s;
        }
        .station-card:hover {
            background: #e9ecef;
            transform: translateX(5px);
        }
        .station-info h3 {
            color: #495057;
            margin-bottom: 5px;
        }
        .station-info p {
            color: #6c757d;
            font-size: 0.9em;
            word-break: break-all;
        }
        .station-actions {
            display: flex;
            gap: 10px;
        }
        .alert {
            padding: 15px;
            border-radius: 8px;
            margin-bottom: 20px;
            display: none;
        }
        .alert.success {
            background: #d4edda;
            color: #155724;
            border: 1px solid #c3e6cb;
        }
        .alert.error {
            background: #f8d7da;
            color: #721c24;
            border: 1px solid #f5c6cb;
        }
        .info-box {
            background: #e7f3ff;
            border-left: 4px solid #667eea;
            padding: 15px;
            border-radius: 8px;
            margin-bottom: 20px;
        }
        .info-box h3 {
            color: #667eea;
            margin-bottom: 10px;
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>🎵 Alarm Clock Radio</h1>
            <p>Control Panel</p>
        </div>
)html";
}

String WebServerModule::getHTMLFooter() {
    return R"html(
    </div>
    <script>
        function showAlert(message, isError = false) {
            const alert = document.getElementById('alert');
            alert.textContent = message;
            alert.className = 'alert ' + (isError ? 'error' : 'success');
            alert.style.display = 'block';
            setTimeout(() => {
                alert.style.display = 'none';
            }, 4000);
        }
    </script>
</body>
</html>
)html";
}

String WebServerModule::getMainHTML() {
    String html = getHTMLHeader();
    
    html += R"html(
        <div class="nav">
            <a href="/" class="active">Play</a>
            <a href="/stations">Manage Stations</a>
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
        </script>
)html";
    
    html += getHTMLFooter();
    return html;
}

String WebServerModule::getStationsHTML() {
    String html = getHTMLHeader();
    
    html += R"html(
        <div class="nav">
            <a href="/">Play</a>
            <a href="/stations" class="active">Manage Stations</a>
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
    
    html += getHTMLFooter();
    return html;
}

String WebServerModule::getSettingsHTML() {
    String html = getHTMLHeader();
    
    // Get current timezone settings
    long gmtOffset = 0;
    long dstOffset = 0;
    if (storage) {
        storage->loadTimezone(gmtOffset, dstOffset);
    }
    
    int gmtHours = gmtOffset / 3600;
    int dstHours = dstOffset / 3600;
    
    html += R"html(
        <div class="nav">
            <a href="/">Play</a>
            <a href="/stations">Manage Stations</a>
            <a href="/settings" class="active">Settings</a>
        </div>
        <div class="content">
            <div id="alert" class="alert"></div>
            
            <div class="info-box">
                <h3>Timezone Configuration</h3>
                <p>Set your timezone offset from GMT and daylight saving time adjustment. Changes require a device restart to take effect.</p>
            </div>
            
            <h2 style="margin-bottom: 20px; color: #495057;">Timezone Settings</h2>
            
            <div class="form-group">
                <label>GMT Offset (hours)</label>
                <select id="gmtOffset">
)html";

    // Generate GMT offset options from -12 to +14
    for (int i = -12; i <= 14; i++) {
        String selected = (i == gmtHours) ? " selected" : "";
        String sign = (i >= 0) ? "+" : "";
        html += "<option value='" + String(i) + "'" + selected + ">GMT" + sign + String(i) + ":00</option>";
    }
    
    html += R"html(
                </select>
            </div>
            
            <div class="form-group">
                <label>Daylight Saving Time Offset (hours)</label>
                <select id="dstOffset">
                    <option value="0")html";
    if (dstHours == 0) html += " selected";
    html += R"html(>No DST (0 hours)</option>
                    <option value="1")html";
    if (dstHours == 1) html += " selected";
    html += R"html(>+1 hour (Standard DST)</option>
                    <option value="2")html";
    if (dstHours == 2) html += " selected";
    html += R"html(>+2 hours</option>
                </select>
            </div>
            
            <button class="btn-primary" onclick="saveTimezone()">💾 Save Timezone</button>
            
            <h2 style="margin-top: 40px; margin-bottom: 20px; color: #495057;">System Information</h2>
            
            <div class="info-box">
)html";

    if (timeModule) {
        html += "<p><strong>Current Time:</strong> " + timeModule->getTimeString() + "</p>";
        html += "<p><strong>Current Date:</strong> " + timeModule->getDateString() + "</p>";
        html += "<p><strong>IP Address:</strong> " + timeModule->getIPAddress() + "</p>";
    }
    
    if (storage) {
        html += "<p><strong>Saved Stations:</strong> " + String(storage->getInternetStationCount()) + " / 10</p>";
    }
    
    html += R"html(
            </div>
            
            <div style="margin-top: 30px; padding-top: 30px; border-top: 2px solid #e9ecef;">
                <h3 style="color: #dc3545; margin-bottom: 15px;">Danger Zone</h3>
                <p style="margin-bottom: 15px; color: #6c757d;">Reset all settings and stations to factory defaults.</p>
                <button class="btn-danger" onclick="factoryReset()">⚠️ Factory Reset</button>
            </div>
        </div>
        
        <script>
            function saveTimezone() {
                const gmtOffset = document.getElementById('gmtOffset').value;
                const dstOffset = document.getElementById('dstOffset').value;
                
                fetch('/save_timezone', {
                    method: 'POST',
                    headers: {'Content-Type': 'application/x-www-form-urlencoded'},
                    body: 'gmtOffset=' + gmtOffset + '&dstOffset=' + dstOffset
                })
                .then(response => response.text())
                .then(data => {
                    showAlert(data);
                })
                .catch(error => {
                    showAlert('Error: ' + error, true);
                });
            }
            
            function factoryReset() {
                if (!confirm('Are you sure you want to reset ALL settings and stations? This cannot be undone!')) {
                    return;
                }
                
                if (!confirm('Really delete everything? Last chance!')) {
                    return;
                }
                
                showAlert('Factory reset not yet implemented in this demo', true);
            }
        </script>
)html";
    
    html += getHTMLFooter();
    return html;
}