#include "WebServerModule.h"

WebServerModule::WebServerModule() : server(nullptr), playCallback(nullptr) {
    server = new WebServer(80);
}

WebServerModule::~WebServerModule() {
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
    server->on("/play", HTTP_POST, [this]() { handlePlay(); });
    server->onNotFound([this]() { handleNotFound(); });
    
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

void WebServerModule::handleRoot() {
    server->send(200, "text/html", getHTML());
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

String WebServerModule::getHTML() {
    String html = R"html(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Alarm Clock Radio</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            max-width: 600px;
            margin: 50px auto;
            padding: 20px;
            background: #f0f0f0;
        }
        .container {
            background: white;
            padding: 30px;
            border-radius: 10px;
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
        }
        h1 {
            color: #333;
            text-align: center;
        }
        input, button {
            width: 100%;
            padding: 10px;
            margin: 10px 0;
            border: 1px solid #ddd;
            border-radius: 5px;
            box-sizing: border-box;
        }
        button {
            background: #4CAF50;
            color: white;
            border: none;
            cursor: pointer;
            font-size: 16px;
        }
        button:hover {
            background: #45a049;
        }
        .status {
            padding: 10px;
            margin: 10px 0;
            border-radius: 5px;
            display: none;
        }
        .success {
            background: #d4edda;
            color: #155724;
            border: 1px solid #c3e6cb;
        }
        .error {
            background: #f8d7da;
            color: #721c24;
            border: 1px solid #f5c6cb;
        }
        .presets {
            margin-top: 30px;
        }
        .preset-btn {
            background: #2196F3;
            margin: 5px 0;
        }
        .preset-btn:hover {
            background: #0b7dda;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🎵 Alarm Clock Radio</h1>
        
        <div id="status" class="status"></div>
        
        <h3>Play Custom Station</h3>
        <input type="text" id="stationName" placeholder="Station Name" value="">
        <input type="text" id="stationUrl" placeholder="Stream URL" value="">
        <button onclick="playStation()">Play</button>
        
        <div class="presets">
            <h3>Quick Presets</h3>
            <button class="preset-btn" onclick="playPreset('BBC Radio 5 Live', 'https://stream.live.vc.bbcmedia.co.uk/bbc_radio_five_live')">BBC Radio 5 Live</button>
            <button class="preset-btn" onclick="playPreset('Veronica', 'https://playerservices.streamtheworld.com/api/livestream-redirect/VERONICAAAC.aac')">Veronica</button>
            <button class="preset-btn" onclick="playPreset('NPO Radio 1', 'https://icecast.omroep.nl/radio1-bb-mp3')">NPO Radio 1</button>
        </div>
    </div>
    
    <script>
        function showStatus(message, isError = false) {
            const status = document.getElementById('status');
            status.textContent = message;
            status.className = 'status ' + (isError ? 'error' : 'success');
            status.style.display = 'block';
            setTimeout(() => {
                status.style.display = 'none';
            }, 3000);
        }
        
        function playStation() {
            const name = document.getElementById('stationName').value;
            const url = document.getElementById('stationUrl').value;
            
            if (!name || !url) {
                showStatus('Please enter both name and URL', true);
                return;
            }
            
            fetch('/play', {
                method: 'POST',
                headers: {'Content-Type': 'application/x-www-form-urlencoded'},
                body: 'name=' + encodeURIComponent(name) + '&url=' + encodeURIComponent(url)
            })
            .then(response => response.text())
            .then(data => {
                showStatus(data);
            })
            .catch(error => {
                showStatus('Error: ' + error, true);
            });
        }
        
        function playPreset(name, url) {
            document.getElementById('stationName').value = name;
            document.getElementById('stationUrl').value = url;
            playStation();
        }
    </script>
</body>
</html>
)html";
    return html;
}