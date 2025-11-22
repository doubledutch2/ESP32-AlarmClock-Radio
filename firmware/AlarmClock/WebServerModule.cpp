// ==================== WebServerModule.cpp ====================
#include "WebServerModule.h"

// Static instance pointer for callbacks
WebServerModule* WebServerModule::instance = nullptr;

WebServerModule::WebServerModule() {
  server = new WebServer(80);
  playCallback = nullptr;
  instance = this;
}

WebServerModule::~WebServerModule() {
  delete server;
}

bool WebServerModule::begin(const char* mdnsName) {
  // Start mDNS
  if (MDNS.begin(mdnsName)) {
    Serial.println("=== mDNS Started ===");
    Serial.printf("Hostname: http://%s.local\n", mdnsName);
    MDNS.addService("http", "tcp", 80);
  } else {
    Serial.println("Error setting up mDNS!");
  }
  
  // Setup routes
  server->on("/", [this]() { this->handleRoot(); });
  server->on("/play", HTTP_POST, [this]() { this->handlePlay(); });
  server->on("/stations", [this]() { this->handleStations(); });
  
  server->begin();
  Serial.println("\n=== Web Server Started ===");
  Serial.println("==========================\n");
  return true;
}

void WebServerModule::handleClient() {
  server->handleClient();
}

void WebServerModule::setPlayCallback(PlayCallback callback) {
  playCallback = callback;
}

void WebServerModule::handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP32 Radio Control</title>
  <style>
    * { margin: 0; padding: 0; box-sizing: border-box; }
    body { font-family: Arial, sans-serif; background: #1a1a2e; color: #eee; padding: 20px; }
    .container { max-width: 600px; margin: 0 auto; }
    h1 { color: #00d4ff; margin-bottom: 20px; text-align: center; }
    .card { background: #16213e; border-radius: 10px; padding: 20px; margin-bottom: 20px; box-shadow: 0 4px 6px rgba(0,0,0,0.3); }
    .card h2 { color: #00d4ff; margin-bottom: 15px; font-size: 1.2em; }
    input[type="text"] { width: 100%; padding: 12px; margin-bottom: 10px; border: 2px solid #00d4ff; background: #0f3460; color: #fff; border-radius: 5px; font-size: 14px; }
    input[type="text"]:focus { outline: none; border-color: #00ffff; }
    button { width: 100%; padding: 12px; background: #00d4ff; color: #000; border: none; border-radius: 5px; font-size: 16px; font-weight: bold; cursor: pointer; transition: 0.3s; }
    button:hover { background: #00ffff; transform: translateY(-2px); }
    button:active { transform: translateY(0); }
    .status { text-align: center; padding: 10px; border-radius: 5px; margin-top: 10px; display: none; }
    .success { background: #28a745; display: block; }
    .error { background: #dc3545; display: block; }
    .station-list { list-style: none; }
    .station-list li { padding: 10px; background: #0f3460; margin-bottom: 8px; border-radius: 5px; border-left: 3px solid #00d4ff; cursor: pointer; transition: 0.3s; }
    .station-list li:hover { background: #1a4d7a; transform: translateX(5px); }
    .station-list li:active { transform: translateX(0); }
    .station-name { font-weight: bold; color: #00ffff; }
    .station-url { font-size: 0.85em; color: #aaa; word-break: break-all; margin-top: 5px; }
  </style>
</head>
<body>
  <div class="container">
    <h1>ESP32 Radio Control</h1>
    
    <div class="card">
      <h2>Play Custom Station</h2>
      <form id="playForm">
        <input type="text" id="stationName" placeholder="Station Name (e.g., BBC Radio 1)" required>
        <input type="text" id="streamURL" placeholder="Stream URL (http:// or https://)" required>
        <button type="submit">Play Now</button>
      </form>
      <div class="status" id="status"></div>
    </div>
    
    <div class="card">
      <h2>Preset Stations</h2>
      <ul class="station-list" id="stationList">Loading...</ul>
    </div>
  </div>
  
  <script>
    let stations = [];
    
    fetch('/stations')
      .then(r => r.json())
      .then(data => {
        stations = data.stations;
        const list = document.getElementById('stationList');
        list.innerHTML = stations.map((s, i) => 
          `<li data-index="${i}"><div class="station-name">${i + 1}. ${s.name}</div><div class="station-url">${s.url}</div></li>`
        ).join('');
        
        document.querySelectorAll('.station-list li').forEach(li => {
          li.addEventListener('click', async () => {
            const index = li.dataset.index;
            const station = stations[index];
            await playStation(station.name, station.url);
          });
        });
      })
      .catch(() => {
        document.getElementById('stationList').innerHTML = '<li>Error loading stations</li>';
      });
    
    async function playStation(name, url) {
      const status = document.getElementById('status');
      
      try {
        const response = await fetch('/play', {
          method: 'POST',
          headers: {'Content-Type': 'application/x-www-form-urlencoded'},
          body: `name=${encodeURIComponent(name)}&url=${encodeURIComponent(url)}`
        });
        
        const result = await response.text();
        status.className = 'status ' + (response.ok ? 'success' : 'error');
        status.textContent = result;
      } catch (error) {
        status.className = 'status error';
        status.textContent = 'Connection error';
      }
    }
    
    document.getElementById('playForm').addEventListener('submit', async (e) => {
      e.preventDefault();
      const name = document.getElementById('stationName').value;
      const url = document.getElementById('streamURL').value;
      
      await playStation(name, url);
      
      if (document.getElementById('status').classList.contains('success')) {
        document.getElementById('playForm').reset();
      }
    });
  </script>
</body>
</html>
)rawliteral";
  
  server->send(200, "text/html", html);
}

void WebServerModule::handlePlay() {
  if (!server->hasArg("url") || !server->hasArg("name")) {
    server->send(400, "text/plain", "Missing parameters");
    return;
  }
  
  String name = server->arg("name");
  String url = server->arg("url");
  
  Serial.println("\n=== Custom Stream Request ===");
  Serial.printf("Name: %s\n", name.c_str());
  Serial.printf("URL: %s\n", url.c_str());
  
  if (playCallback) {
    playCallback(name.c_str(), url.c_str());
    server->send(200, "text/plain", "Now playing: " + name);
  } else {
    server->send(500, "text/plain", "Play callback not set");
  }
}

void WebServerModule::handleStations() {
  // This will be populated by the main code
  String json = "{\"stations\":[]}";
  server->send(200, "application/json", json);
}

