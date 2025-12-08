#include "WebServerAlarms.h"
#include <LittleFS.h>

WebServerAlarms::WebServerAlarms(WebServer* srv, StorageModule* stor, AudioModule* aud, FMRadioModule* fm)
    : server(srv), storage(stor), audio(aud), fmRadio(fm), stationList(nullptr), stationCount(0) {
}

void WebServerAlarms::setStationList(InternetRadioStation* stations, int count) {
    stationList = stations;
    stationCount = count;
}

void WebServerAlarms::setupRoutes() {
    if (!server) return;
    
    server->on("/alarms", [this]() { handleAlarms(); });
    server->on("/save_alarm", HTTP_POST, [this]() { handleSaveAlarm(); });
    server->on("/test_alarm", HTTP_POST, [this]() { handleTestAlarm(); });
    server->on("/list_mp3", [this]() { handleListMP3(); });
    
    Serial.println("WebServerAlarms: Routes registered");
}

void WebServerAlarms::handleAlarms() {
    server->send(200, "text/html", getAlarmsHTML());
}

void WebServerAlarms::handleSaveAlarm() {
    if (!storage || !server->hasArg("index")) {
        server->send(400, "text/plain", "Missing parameters");
        return;
    }
    
    int index = server->arg("index").toInt();
    if (index < 0 || index >= MAX_ALARMS) {
        server->send(400, "text/plain", "Invalid alarm index");
        return;
    }
    
    AlarmConfig alarm;
    alarm.enabled = server->hasArg("enabled") && server->arg("enabled") == "1";
    alarm.hour = server->arg("hour").toInt();
    alarm.minute = server->arg("minute").toInt();
    alarm.repeatMode = (AlarmRepeat)server->arg("repeat").toInt();
    alarm.soundType = (AlarmSoundType)server->arg("soundType").toInt();
    
    // Sound source parameters
    alarm.stationIndex = server->arg("stationIndex").toInt();
    alarm.fmFrequency = server->arg("fmFreq").toFloat();
    alarm.mp3File = server->arg("mp3File");
    
    // Don't modify last triggered date when saving
    AlarmConfig existing;
    storage->loadAlarm(index, existing);
    alarm.lastYear = existing.lastYear;
    alarm.lastMonth = existing.lastMonth;
    alarm.lastDay = existing.lastDay;
    
    if (storage->saveAlarm(index, alarm)) {
        server->send(200, "text/plain", "Alarm saved successfully");
    } else {
        server->send(500, "text/plain", "Failed to save alarm");
    }
}

void WebServerAlarms::handleTestAlarm() {
    if (!audio || !server->hasArg("soundType")) {
        server->send(400, "text/plain", "Missing parameters");
        return;
    }
    
    int soundType = server->arg("soundType").toInt();
    
    switch (soundType) {
        case SOUND_INTERNET_RADIO:
            if (server->hasArg("stationIndex")) {
                int index = server->arg("stationIndex").toInt();
                audio->playStation(index);
                server->send(200, "text/plain", "Testing internet radio");
            }
            break;
            
        case SOUND_FM_RADIO:
            if (server->hasArg("fmFreq") && fmRadio) {
                float freq = server->arg("fmFreq").toFloat();
                fmRadio->setFrequency(freq);
                server->send(200, "text/plain", "Testing FM radio");
            }
            break;
            
        case SOUND_MP3_FILE:
            if (server->hasArg("mp3File")) {
                String file = server->arg("mp3File");
                if (audio->playMP3File(file.c_str(), false)) {
                    server->send(200, "text/plain", "Testing MP3 file");
                } else {
                    server->send(500, "text/plain", "Failed to play MP3");
                }
            }
            break;
            
        default:
            server->send(400, "text/plain", "Invalid sound type");
    }
}

void WebServerAlarms::handleListMP3() {
    String json = "[";
    
    // List MP3 files from /mp3/ directory
    if (LittleFS.begin()) {
        File root = LittleFS.open("/mp3");
        if (root && root.isDirectory()) {
            File file = root.openNextFile();
            bool first = true;
            
            while (file) {
                if (!file.isDirectory()) {
                    String name = String(file.name());
                    if (name.endsWith(".mp3") || name.endsWith(".MP3")) {
                        if (!first) json += ",";
                        json += "\"" + name + "\"";
                        first = false;
                    }
                }
                file = root.openNextFile();
            }
        }
    }
    
    json += "]";
    server->send(200, "application/json", json);
}

String WebServerAlarms::getHTMLHeader() {
    return R"html(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Alarm Management</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Arial, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            padding: 20px;
        }
        .container {
            max-width: 900px;
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
        .content {
            padding: 30px;
        }
        .alarm-card {
            background: #f8f9fa;
            padding: 20px;
            border-radius: 10px;
            margin-bottom: 20px;
            border: 2px solid #e9ecef;
        }
        .alarm-card.enabled {
            border-color: #28a745;
            background: #d4edda;
        }
        .form-group {
            margin-bottom: 15px;
        }
        .form-group label {
            display: block;
            margin-bottom: 5px;
            color: #495057;
            font-weight: 600;
        }
        .form-group input, .form-group select {
            width: 100%;
            padding: 10px;
            border: 2px solid #e9ecef;
            border-radius: 8px;
            font-size: 16px;
        }
        button {
            padding: 10px 20px;
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
        .btn-success {
            background: #28a745;
            color: white;
        }
        .btn-warning {
            background: #ffc107;
            color: #000;
        }
        .btn-danger {
            background: #dc3545;
            color: white;
        }
        .alarm-header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 15px;
        }
        .alarm-time {
            font-size: 2em;
            font-weight: bold;
            color: #495057;
        }
        .toggle {
            display: inline-block;
            position: relative;
            width: 60px;
            height: 34px;
        }
        .toggle input {
            opacity: 0;
            width: 0;
            height: 0;
        }
        .slider {
            position: absolute;
            cursor: pointer;
            top: 0;
            left: 0;
            right: 0;
            bottom: 0;
            background-color: #ccc;
            transition: .4s;
            border-radius: 34px;
        }
        .slider:before {
            position: absolute;
            content: "";
            height: 26px;
            width: 26px;
            left: 4px;
            bottom: 4px;
            background-color: white;
            transition: .4s;
            border-radius: 50%;
        }
        input:checked + .slider {
            background-color: #28a745;
        }
        input:checked + .slider:before {
            transform: translateX(26px);
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>⏰ Alarm Management</h1>
        </div>
)html";
}

String WebServerAlarms::getHTMLFooter() {
    return R"html(
    </div>
</body>
</html>
)html";
}

String WebServerAlarms::getAlarmsHTML() {
    String html = getHTMLHeader();
    html += "<div class='content'>";
    
    // Load and display all alarms
    for (int i = 0; i < MAX_ALARMS; i++) {
        AlarmConfig alarm;
        if (storage) {
            storage->loadAlarm(i, alarm);
        }
        
        String cardClass = alarm.enabled ? "alarm-card enabled" : "alarm-card";
        html += "<div class='" + cardClass + "'>";
        html += "<div class='alarm-header'>";
        html += "<h2>Alarm " + String(i + 1) + "</h2>";
        html += "<label class='toggle'>";
        html += "<input type='checkbox' id='enabled_" + String(i) + "' " + 
                String(alarm.enabled ? "checked" : "") + " onchange='toggleAlarm(" + String(i) + ")'>";
        html += "<span class='slider'></span>";
        html += "</label>";
        html += "</div>";
        
        html += "<div class='alarm-time'>" + String(alarm.hour) + ":" + 
                String(alarm.minute < 10 ? "0" : "") + String(alarm.minute) + "</div>";
        
        html += "<div class='form-group'>";
        html += "<label>Time</label>";
        html += "<input type='time' id='time_" + String(i) + "' value='" + 
                String(alarm.hour < 10 ? "0" : "") + String(alarm.hour) + ":" +
                String(alarm.minute < 10 ? "0" : "") + String(alarm.minute) + "'>";
        html += "</div>";
        
        html += "<div class='form-group'>";
        html += "<label>Repeat</label>";
        html += "<select id='repeat_" + String(i) + "'>";
        html += "<option value='0'" + String(alarm.repeatMode == ALARM_ONCE ? " selected" : "") + ">Once</option>";
        html += "<option value='1'" + String(alarm.repeatMode == ALARM_DAILY ? " selected" : "") + ">Daily</option>";
        html += "<option value='2'" + String(alarm.repeatMode == ALARM_WEEKDAYS ? " selected" : "") + ">Weekdays</option>";
        html += "<option value='3'" + String(alarm.repeatMode == ALARM_WEEKENDS ? " selected" : "") + ">Weekends</option>";
        html += "</select>";
        html += "</div>";
        
        html += "<div class='form-group'>";
        html += "<label>Sound Type</label>";
        html += "<select id='soundType_" + String(i) + "' onchange='updateSoundOptions(" + String(i) + ")'>";
        html += "<option value='0'" + String(alarm.soundType == SOUND_INTERNET_RADIO ? " selected" : "") + ">Internet Radio</option>";
        html += "<option value='1'" + String(alarm.soundType == SOUND_FM_RADIO ? " selected" : "") + ">FM Radio</option>";
        html += "<option value='2'" + String(alarm.soundType == SOUND_MP3_FILE ? " selected" : "") + ">MP3 File</option>";
        html += "</select>";
        html += "</div>";
        
        // Internet Radio options
        html += "<div id='radioOptions_" + String(i) + "' class='form-group' style='display:" + 
                String(alarm.soundType == SOUND_INTERNET_RADIO ? "block" : "none") + "'>";
        html += "<label>Station</label>";
        html += "<select id='station_" + String(i) + "'>";
        for (int j = 0; j < stationCount; j++) {
            html += "<option value='" + String(j) + "'" + 
                    String(alarm.stationIndex == j ? " selected" : "") + ">" + 
                    stationList[j].name + "</option>";
        }
        html += "</select>";
        html += "</div>";
        
        // FM Radio options
        html += "<div id='fmOptions_" + String(i) + "' class='form-group' style='display:" + 
                String(alarm.soundType == SOUND_FM_RADIO ? "block" : "none") + "'>";
        html += "<label>FM Frequency (MHz)</label>";
        html += "<input type='number' id='fmFreq_" + String(i) + "' min='87.0' max='108.0' step='0.1' value='" + 
                String(alarm.fmFrequency) + "'>";
        html += "</div>";
        
        // MP3 options
        html += "<div id='mp3Options_" + String(i) + "' class='form-group' style='display:" + 
                String(alarm.soundType == SOUND_MP3_FILE ? "block" : "none") + "'>";
        html += "<label>MP3 File</label>";
        html += "<input type='text' id='mp3File_" + String(i) + "' value='" + alarm.mp3File + "' placeholder='alarm.mp3'>";
        html += "</div>";
        
        html += "<button class='btn-primary' onclick='saveAlarm(" + String(i) + ")'>💾 Save</button>";
        html += "<button class='btn-success' onclick='testAlarm(" + String(i) + ")'>▶ Test</button>";
        html += "</div>";
    }
    
    html += R"html(
        <script>
            function updateSoundOptions(index) {
                const soundType = document.getElementById('soundType_' + index).value;
                document.getElementById('radioOptions_' + index).style.display = soundType == '0' ? 'block' : 'none';
                document.getElementById('fmOptions_' + index).style.display = soundType == '1' ? 'block' : 'none';
                document.getElementById('mp3Options_' + index).style.display = soundType == '2' ? 'block' : 'none';
            }
            
            function toggleAlarm(index) {
                saveAlarm(index);
            }
            
            function saveAlarm(index) {
                const time = document.getElementById('time_' + index).value.split(':');
                const enabled = document.getElementById('enabled_' + index).checked ? '1' : '0';
                const repeat = document.getElementById('repeat_' + index).value;
                const soundType = document.getElementById('soundType_' + index).value;
                
                let params = 'index=' + index + '&enabled=' + enabled + '&hour=' + time[0] + '&minute=' + time[1];
                params += '&repeat=' + repeat + '&soundType=' + soundType;
                
                if (soundType == '0') {
                    params += '&stationIndex=' + document.getElementById('station_' + index).value;
                } else if (soundType == '1') {
                    params += '&fmFreq=' + document.getElementById('fmFreq_' + index).value;
                } else if (soundType == '2') {
                    params += '&mp3File=' + encodeURIComponent(document.getElementById('mp3File_' + index).value);
                }
                
                fetch('/save_alarm', {
                    method: 'POST',
                    headers: {'Content-Type': 'application/x-www-form-urlencoded'},
                    body: params
                })
                .then(response => response.text())
                .then(data => {
                    alert(data);
                    location.reload();
                })
                .catch(error => alert('Error: ' + error));
            }
            
            function testAlarm(index) {
                const soundType = document.getElementById('soundType_' + index).value;
                let params = 'soundType=' + soundType;
                
                if (soundType == '0') {
                    params += '&stationIndex=' + document.getElementById('station_' + index).value;
                } else if (soundType == '1') {
                    params += '&fmFreq=' + document.getElementById('fmFreq_' + index).value;
                } else if (soundType == '2') {
                    params += '&mp3File=' + encodeURIComponent(document.getElementById('mp3File_' + index).value);
                }
                
                fetch('/test_alarm', {
                    method: 'POST',
                    headers: {'Content-Type': 'application/x-www-form-urlencoded'},
                    body: params
                })
                .then(response => response.text())
                .then(data => alert(data))
                .catch(error => alert('Error: ' + error));
            }
        </script>
    )html";
    
    html += "</div>";
    html += getHTMLFooter();
    return html;
}
