#include "WebServerHTML.h"
#include "FeatureFlags.h"
#include "StorageModule.h"
#include "TimeModule.h"
#include "AudioModule.h"
#include "DisplayILI9341.h"

String WebServerHTML::getHTMLHeader() {
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
            overflow-x: auto;
        }
        .nav a {
            flex: 1;
            min-width: 100px;
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
        .btn-warning {
            background: #ffc107;
            color: #000;
        }
        .btn-warning:hover {
            background: #e0a800;
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
        .slider-container {
            margin: 30px 0;
        }
        .slider {
            width: 100%;
            height: 8px;
            border-radius: 5px;
            background: #e9ecef;
            outline: none;
            -webkit-appearance: none;
        }
        .slider::-webkit-slider-thumb {
            -webkit-appearance: none;
            appearance: none;
            width: 25px;
            height: 25px;
            border-radius: 50%;
            background: #667eea;
            cursor: pointer;
        }
        .slider::-moz-range-thumb {
            width: 25px;
            height: 25px;
            border-radius: 50%;
            background: #667eea;
            cursor: pointer;
        }
        .slider-value {
            display: inline-block;
            min-width: 50px;
            text-align: center;
            font-size: 1.5em;
            font-weight: bold;
            color: #667eea;
            margin-left: 15px;
        }
        .checkbox-group {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 15px;
            margin: 20px 0;
        }
        .checkbox-item {
            display: flex;
            align-items: center;
            padding: 12px;
            background: #f8f9fa;
            border-radius: 8px;
            transition: background 0.3s;
        }
        .checkbox-item:hover {
            background: #e9ecef;
        }
        .checkbox-item input[type="checkbox"] {
            width: 20px;
            height: 20px;
            margin-right: 10px;
            cursor: pointer;
        }
        .checkbox-item label {
            cursor: pointer;
            user-select: none;
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

String WebServerHTML::getHTMLFooter() {
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

String WebServerHTML::getControlHTML(AudioModule* audioModule, DisplayILI9341* displayModule, TimeModule* timeModule) {
    String html = getHTMLHeader();
    
    // Get current values
    int currentVolume = audioModule ? audioModule->getCurrentVolume() : 3;
    int maxVolume = audioModule ? audioModule->getMaxVolume() : 21;
    int currentBrightness = displayModule ? displayModule->getBrightness() : 200;
    
    html += R"html(
        <div class="nav">
            <a href="/">Play</a>
            <a href="/control" class="active">Control</a>
            <a href="/stations">Stations</a>
            <a href="/alarms">Alarms</a>
            <a href="/settings">Settings</a>
        </div>
        <div class="content">
            <div id="alert" class="alert"></div>
            
            <div class="info-box">
                <h3>Volume & Brightness Control</h3>
                <p>Adjust volume and screen brightness. Settings are saved automatically.</p>
            </div>
            
            <div class="slider-container">
                <h2 style="color: #495057; margin-bottom: 15px;">🔊 Volume</h2>
                <div style="display: flex; align-items: center;">
                    <input type="range" class="slider" id="volumeSlider" 
                           min="0" max=")html" + String(maxVolume) + R"html(" 
                           value=")html" + String(currentVolume) + R"html(" 
                           oninput="updateVolumeDisplay(this.value)">
                    <span class="slider-value" id="volumeValue">)html" + String(currentVolume) + R"html(</span>
                </div>
                <button class="btn-primary" onclick="saveVolume()" style="margin-top: 15px;">💾 Save Volume</button>
            </div>
            
            <div class="slider-container">
                <h2 style="color: #495057; margin-bottom: 15px;">💡 Brightness</h2>
                <div style="display: flex; align-items: center;">
                    <input type="range" class="slider" id="brightnessSlider" 
                           min="0" max="255" 
                           value=")html" + String(currentBrightness) + R"html(" 
                           oninput="updateBrightnessDisplay(this.value)">
                    <span class="slider-value" id="brightnessValue">)html" + String(currentBrightness) + R"html(</span>
                </div>
                <button class="btn-primary" onclick="saveBrightness()" style="margin-top: 15px;">💾 Save Brightness</button>
            </div>
            
            <div class="info-box" style="margin-top: 40px;">
                <h3>Current Status</h3>
)html";

    if (audioModule) {
        html += "<p><strong>Now Playing:</strong> " + audioModule->getCurrentStationName() + "</p>";
        html += "<p><strong>Audio Status:</strong> " + String(audioModule->getIsPlaying() ? "Playing" : "Stopped") + "</p>";
    }
    
    if (timeModule) {
        html += "<p><strong>Time:</strong> " + timeModule->getTimeString() + "</p>";
    }
    
    html += R"html(
            </div>
        </div>
        <script>
            function updateVolumeDisplay(value) {
                document.getElementById('volumeValue').textContent = value;
            }
            
            function updateBrightnessDisplay(value) {
                document.getElementById('brightnessValue').textContent = value;
            }
            
            function saveVolume() {
                const volume = document.getElementById('volumeSlider').value;
                
                fetch('/set_volume', {
                    method: 'POST',
                    headers: {'Content-Type': 'application/x-www-form-urlencoded'},
                    body: 'volume=' + volume
                })
                .then(response => response.text())
                .then(data => {
                    showAlert('Volume saved: ' + volume);
                })
                .catch(error => {
                    showAlert('Error: ' + error, true);
                });
            }
            
            function saveBrightness() {
                const brightness = document.getElementById('brightnessSlider').value;
                
                fetch('/set_brightness', {
                    method: 'POST',
                    headers: {'Content-Type': 'application/x-www-form-urlencoded'},
                    body: 'brightness=' + brightness
                })
                .then(response => response.text())
                .then(data => {
                    showAlert('Brightness saved: ' + brightness);
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

String WebServerHTML::getSettingsHTML(StorageModule* storage, TimeModule* timeModule) {
    String html = getHTMLHeader();
    
    // Get current timezone settings
    long gmtOffset = 0;
    long dstOffset = 0;
    if (storage) {
        storage->loadTimezone(gmtOffset, dstOffset);
    }
    
    int gmtHours = gmtOffset / 3600;
    int dstHours = dstOffset / 3600;
    
    // Get current feature flags
    FeatureFlags flags;
    if (storage) {
        storage->loadFeatureFlags(flags);
    }
    
    html += R"html(
        <div class="nav">
            <a href="/">Play</a>
            <a href="/control">Control</a>
            <a href="/stations">Stations</a>
            <a href="/alarms">Alarms</a>
            <a href="/settings" class="active">Settings</a>
        </div>
        <div class="content">
            <div id="alert" class="alert"></div>
            
            <!-- Feature Flags Section -->
            <div class="info-box">
                <h3>Feature Configuration</h3>
                <p>Enable or disable hardware features. <strong>Restart required</strong> after changes.</p>
            </div>
            
            <h2 style="margin-bottom: 20px; color: #495057;">Hardware Features</h2>
            
            <form id="featuresForm">
                <div class="checkbox-group">
                    <div class="checkbox-item">
                        <input type="checkbox" id="touchscreen" name="touchscreen" )html" + 
                        String(flags.enableTouchScreen ? "checked" : "") + R"html(>
                        <label for="touchscreen">TouchScreen</label>
                    </div>
                    <div class="checkbox-item">
                        <input type="checkbox" id="buttons" name="buttons" )html" + 
                        String(flags.enableButtons ? "checked" : "") + R"html(>
                        <label for="buttons">Buttons</label>
                    </div>
                    <div class="checkbox-item">
                        <input type="checkbox" id="draw" name="draw" )html" + 
                        String(flags.enableDraw ? "checked" : "") + R"html(>
                        <label for="draw">Draw</label>
                    </div>
                    <div class="checkbox-item">
                        <input type="checkbox" id="audio" name="audio" )html" + 
                        String(flags.enableAudio ? "checked" : "") + R"html(>
                        <label for="audio">Audio</label>
                    </div>
                    <div class="checkbox-item">
                        <input type="checkbox" id="stereo" name="stereo" )html" + 
                        String(flags.enableStereo ? "checked" : "") + R"html(>
                        <label for="stereo">Stereo</label>
                    </div>
                    <div class="checkbox-item">
                        <input type="checkbox" id="led" name="led" )html" + 
                        String(flags.enableLED ? "checked" : "") + R"html(>
                        <label for="led">LED</label>
                    </div>
                    <div class="checkbox-item">
                        <input type="checkbox" id="alarms" name="alarms" )html" + 
                        String(flags.enableAlarms ? "checked" : "") + R"html(>
                        <label for="alarms">Alarms</label>
                    </div>
                    <div class="checkbox-item">
                        <input type="checkbox" id="web" name="web" )html" + 
                        String(flags.enableWeb ? "checked" : "") + R"html(>
                        <label for="web">Web Server</label>
                    </div>
                    <div class="checkbox-item">
                        <input type="checkbox" id="fmradio" name="fmradio" )html" + 
                        String(flags.enableFMRadio ? "checked" : "") + R"html(>
                        <label for="fmradio">FM Radio</label>
                    </div>
                    <div class="checkbox-item">
                        <input type="checkbox" id="pram" name="pram" )html" + 
                        String(flags.enablePRAM ? "checked" : "") + R"html(>
                        <label for="pram">PSRAM</label>
                    </div>
                    <div class="checkbox-item">
                        <input type="checkbox" id="i2cscan" name="i2cscan" )html" + 
                        String(flags.enableI2CScan ? "checked" : "") + R"html(>
                        <label for="i2cscan">I2C Scan</label>
                    </div>
                </div>
                
                <button type="button" class="btn-primary" onclick="saveFeatures()">💾 Save Features</button>
            </form>
            
            <!-- Timezone Section -->
            <h2 style="margin-top: 40px; margin-bottom: 20px; color: #495057;">Timezone Settings</h2>
            
            <div class="info-box">
                <h3>Timezone Configuration</h3>
                <p>Set your timezone offset from GMT and daylight saving time adjustment. <strong>Restart required</strong> after changes.</p>
            </div>
            
            <div class="form-group">
                <label>GMT Offset (hours)</label>
                <select id="gmtOffset">)html";

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
            
            <!-- System Information -->
            <h2 style="margin-top: 40px; margin-bottom: 20px; color: #495057;">System Information</h2>
            
            <div class="info-box">)html";

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
        </div>
        
        <script>
            function saveFeatures() {
                const form = document.getElementById('featuresForm');
                const formData = new FormData(form);
                const params = new URLSearchParams(formData).toString();
                
                fetch('/save_features', {
                    method: 'POST',
                    headers: {'Content-Type': 'application/x-www-form-urlencoded'},
                    body: params
                })
                .then(response => response.text())
                .then(data => {
                    showAlert(data);
                })
                .catch(error => {
                    showAlert('Error: ' + error, true);
                });
            }
            
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
        </script>
)html";
    
    html += getHTMLFooter();
    return html;
}
