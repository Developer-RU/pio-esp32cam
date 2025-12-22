/**
 * @file HtmlPages.cpp
 * @brief Реализация генерации HTML страниц
 */

#include "Web/HtmlPages.hpp"
#include "Config/Config.hpp"
#include <WiFi.h>

// Внешние объявления
extern bool camera_initialized;
extern bool sd_initialized;
extern Settings settings;

/**
 * @brief Генерация HTML главной страницы
 */
String getMainPage()
{
    String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>Car Detector</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: Arial; margin: 20px; background: #f0f0f0; }
        .nav { background: white; padding: 15px; border-radius: 10px; margin-bottom: 20px; }
        .nav a { margin: 0 15px; text-decoration: none; color: #333; font-weight: bold; }
        .card { background: white; padding: 20px; border-radius: 10px; margin-bottom: 20px; }
        .btn { width: 100%; padding: 8px; margin: 5px 0; background: #007bff; color: white; border: 1px solid #ddd; border-radius: 5px; cursor: pointer; }
        input, select { width: 100%; padding: 8px; margin: 5px 0; border: 1px solid #ddd; border-radius: 5px; }
        .status { padding: 10px; border-radius: 5px; margin: 5px 0; }
        .status-ok { background: #d4edda; color: #155724; }
        .status-error { background: #f8d7da; color: #721c24; }
    </style>
</head>
<body>
    <div class="card">
        <h3>Quick Navigation</h3>
        <button class="btn" onclick="location.href='/'">Home Page</button><br>
        <button class="btn" onclick="location.href='/detection'">Detection Settings</button><br>
        <button class="btn" onclick="location.href='/wifi'">WiFi Settings</button><br>
        <button class="btn" onclick="location.href='/roi'">ROI Settings</button><br>
        <button class="btn" onclick="location.href='/list_photos'">Photos info</button>
    </div>
    
    <div class="card">
        <h2>Car Detection System</h2>
        <div class="status )rawliteral" + String(camera_initialized ? "status-ok" : "status-error") + R"rawliteral(">
            Camera: )rawliteral" + String(camera_initialized ? "OK" : "ERROR") + R"rawliteral(
        </div>
        <div class="status )rawliteral" + String(sd_initialized ? "status-ok" : "status-error") + R"rawliteral(">
            SD Card: )rawliteral" + String(sd_initialized ? "OK" : "ERROR") + R"rawliteral(
        </div>
        <p>IP Address: )rawliteral" + WiFi.softAPIP().toString() + R"rawliteral(</p>
        <p>Connected clients: )rawliteral" + String(WiFi.softAPgetStationNum()) + R"rawliteral(</p>
    </div>
</body>
</html>
)rawliteral";
    return html;
}

/**
 * @brief Генерация HTML страницы настроек детекции
 */
String getDetectionSettingsPage()
{
    String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>Car Detector</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: Arial; margin: 20px; background: #f0f0f0; }
        .nav { background: white; padding: 15px; border-radius: 10px; margin-bottom: 20px; }
        .nav a { margin: 0 15px; text-decoration: none; color: #333; font-weight: bold; }
        .card { background: white; padding: 20px; border-radius: 10px; margin-bottom: 20px; }
        .btn { width: 100%; padding: 8px; margin: 5px 0; background: #007bff; color: white; border: 1px solid #ddd; border-radius: 5px; cursor: pointer; }
        input, select { width: 100%; padding: 8px; margin: 5px 0; border: 1px solid #ddd; border-radius: 5px; }
        .status { padding: 10px; border-radius: 5px; margin: 5px 0; }
        .status-ok { background: #d4edda; color: #155724; }
        .status-error { background: #f8d7da; color: #721c24; }
    </style>
</head>
<body>
    <div class="card">
        <h3>Quick Navigation</h3>
        <button class="btn" onclick="location.href='/'">Home Page</button><br>
        <button class="btn" onclick="location.href='/detection'">Detection Settings</button><br>
        <button class="btn" onclick="location.href='/wifi'">WiFi Settings</button><br>
        <button class="btn" onclick="location.href='/roi'">ROI Settings</button><br>
        <button class="btn" onclick="location.href='/list_photos'">Photos info</button>
    </div>
    
    <div class="card">
        <h2>Detection Sensitivity Settings</h2>
        <form id="detectionForm">
            <label>Distance (0-400): <span class="value-display" id="distanceValue">)rawliteral" + 
                String(settings.distance) + R"rawliteral(</span></label>
            <input type="range" class="slider" id="distance" min="0" max="400" value=")rawliteral" + 
                String(settings.distance) + R"rawliteral("></input><br>
            
            <label>Interval detection (0-10000):</label>
            <input type="number" id="interval" step="100" value=")rawliteral" + 
                String(settings.interval) + R"rawliteral(" min="0" max="10000">
            
            <label>Threshold (0-255): <span class="value-display" id="thresholdValue">)rawliteral" + 
                String(settings.threshold) + R"rawliteral(</span></label>
            <input type="range" class="slider" id="threshold" min="0" max="255" value=")rawliteral" + 
                String(settings.threshold) + R"rawliteral("></input><br>
            
            <label>Minimum Area:</label>
            <input type="number" id="area" value=")rawliteral" + 
                String(settings.area) + R"rawliteral(" min="1" max="10000">
            
            <label>Dark Pixel Min Ratio (0-1):</label>
            <input type="number" id="dark_min" step="0.01" value=")rawliteral" + 
                String(settings.dark_min) + R"rawliteral(" min="0" max="1">
            
            <label>Dark Pixel Max Ratio (0-1):</label>
            <input type="number" id="dark_max" step="0.01" value=")rawliteral" + 
                String(settings.dark_max) + R"rawliteral(" min="0" max="1">
            
            <label>Texture Sensitivity:</label>
            <input type="number" id="texture" value=")rawliteral" + 
                String(settings.texture) + R"rawliteral(" min="0" max="10000">
            
            <label>Max Files:</label>
            <input type="number" id="max_files" value=")rawliteral" + 
                String(settings.max_files) + R"rawliteral(" min="1" max="1000">
            
            <button type="button" class="btn" onclick="saveSettings()">Save Settings</button>
        </form>
    </div>

    <script>
        document.getElementById('distance').oninput = function() {
            document.getElementById('distanceValue').textContent = this.value;
        }

        document.getElementById('threshold').oninput = function() {
            document.getElementById('thresholdValue').textContent = this.value;
        }
        
        async function saveSettings() {
            const formData = new FormData();
            formData.append('distance', document.getElementById('distance').value);
            formData.append('interval', document.getElementById('interval').value);
            formData.append('threshold', document.getElementById('threshold').value);
            formData.append('area', document.getElementById('area').value);
            formData.append('dark_min', document.getElementById('dark_min').value);
            formData.append('dark_max', document.getElementById('dark_max').value);
            formData.append('texture', document.getElementById('texture').value);
            formData.append('max_files', document.getElementById('max_files').value);
            
            try {
                const response = await fetch('/save_detection', { method: 'POST', body: formData });
                if (response.ok) {
                    alert('Settings saved successfully!');
                } else {
                    alert('Error saving settings!');
                }
            } catch (error) {
                alert('Error saving settings: ' + error);
            }
        }
    </script>
</body>
</html>
)rawliteral";
    return html;
}

/**
 * @brief Генерация HTML страницы настроек Wi-Fi
 */
String getWifiSettingsPage()
{
    String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>Car Detector</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: Arial; margin: 20px; background: #f0f0f0; }
        .nav { background: white; padding: 15px; border-radius: 10px; margin-bottom: 20px; }
        .nav a { margin: 0 15px; text-decoration: none; color: #333; font-weight: bold; }
        .card { background: white; padding: 20px; border-radius: 10px; margin-bottom: 20px; }
        .btn { width: 100%; padding: 8px; margin: 5px 0; background: #007bff; color: white; border: 1px solid #ddd; border-radius: 5px; cursor: pointer; }
        input, select { width: 100%; padding: 8px; margin: 5px 0; border: 1px solid #ddd; border-radius: 5px; }
        .status { padding: 10px; border-radius: 5px; margin: 5px 0; }
        .status-ok { background: #d4edda; color: #155724; }
        .status-error { background: #f8d7da; color: #721c24; }
    </style>
</head>
<body>
    <div class="card">
        <h3>Quick Navigation</h3>
        <button class="btn" onclick="location.href='/'">Home Page</button><br>
        <button class="btn" onclick="location.href='/detection'">Detection Settings</button><br>
        <button class="btn" onclick="location.href='/wifi'">WiFi Settings</button><br>
        <button class="btn" onclick="location.href='/roi'">ROI Settings</button><br>
        <button class="btn" onclick="location.href='/list_photos'">Photos info</button>
    </div>
    
    <div class="card">
        <h2>WiFi Access Point Settings</h2>
        <p><strong>Current AP:</strong> )rawliteral" + settings.ap_ssid + R"rawliteral(</p>
        <p><strong>Current Password:</strong> )rawliteral" + settings.ap_password + R"rawliteral(</p>
        
        <form id="wifiForm">
            <label>SSID:</label>
            <input type="text" id="ssid" value=")rawliteral" + settings.ap_ssid + R"rawliteral(" required>
            
            <label>Password:</label>
            <input type="password" id="password" value=")rawliteral" + settings.ap_password + R"rawliteral(" required minlength="8">
            
            <button type="button" class="btn" onclick="saveSettings()">Save WiFi Settings</button>
        </form>
        
        <p><em>Note: Changes will take effect after reboot</em></p>
    </div>

    <script>
        async function saveSettings() {
            const formData = new FormData();
            formData.append('ssid', document.getElementById('ssid').value);
            formData.append('password', document.getElementById('password').value);
            
            try {
                const response = await fetch('/save_wifi', { method: 'POST', body: formData });
                if (response.ok) {
                    alert('WiFi settings saved! Please reboot the device.');
                } else {
                    alert('Error saving WiFi settings!');
                }
            } catch (error) {
                alert('Error saving WiFi settings: ' + error);
            }
        }
    </script>
</body>
</html>
)rawliteral";
    return html;
}

/**
 * @brief Генерация HTML страницы настроек ROI
 */
String getROISettingsPage()
{
    String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>Car Detector</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: Arial; margin: 20px; background: #f0f0f0; }
        .nav { background: white; padding: 15px; border-radius: 10px; margin-bottom: 20px; }
        .nav a { margin: 0 15px; text-decoration: none; color: #333; font-weight: bold; }
        .card { background: white; padding: 20px; border-radius: 10px; margin-bottom: 20px; }
        .btn { width: 100%; padding: 8px; margin: 5px 0; background: #007bff; color: white; border: 1px solid #ddd; border-radius: 5px; cursor: pointer; }
        input, select { width: 100%; padding: 8px; margin: 5px 0; border: 1px solid #ddd; border-radius: 5px; }
        .status { padding: 10px; border-radius: 5px; margin: 5px 0; }
        .status-ok { background: #d4edda; color: #155724; }
        .status-error { background: #f8d7da; color: #721c24; }
    </style>
</head>
<body>
    <div class="card">
        <h3>Quick Navigation</h3>
        <button class="btn" onclick="location.href='/'">Home Page</button><br>
        <button class="btn" onclick="location.href='/detection'">Detection Settings</button><br>
        <button class="btn" onclick="location.href='/wifi'">WiFi Settings</button><br>
        <button class="btn" onclick="location.href='/roi'">ROI Settings</button><br>
        <button class="btn" onclick="location.href='/list_photos'">Photos info</button>
    </div>
    
    <div class="card">
        <h2>Region of Interest (ROI) Settings</h2>
        <p>Define the rectangular area in the center of the image that will be analyzed for car detection.</p>
        
        <form id="roiForm">
            <label>ROI Width (pixels):</label>
            <input type="number" id="width" value=")rawliteral" + String(settings.roi_width) + R"rawliteral(" min="1" max="160">
            
            <label>ROI Height (pixels):</label>
            <input type="number" id="height" value=")rawliteral" + String(settings.roi_height) + R"rawliteral(" min="1" max="120">
            
            <label>ROI X Offset:</label>
            <input type="number" id="x" value=")rawliteral" + String(settings.roi_x) + R"rawliteral(" min="0" max="160">
            
            <label>ROI Y Offset:</label>
            <input type="number" id="y" value=")rawliteral" + String(settings.roi_y) + R"rawliteral(" min="0" max="120">
            
            <button type="button" class="btn" onclick="saveSettings()">Save ROI Settings</button>
        </form>
        
        <p><em>Note: Detection image is 160x120 pixels. ROI must fit within these dimensions.</em></p>
        <p><strong>Current ROI:</strong> X=)rawliteral" + String(settings.roi_x) + R"rawliteral(, Y=)rawliteral" + 
           String(settings.roi_y) + R"rawliteral(, Width=)rawliteral" + String(settings.roi_width) + 
           R"rawliteral(, Height=)rawliteral" + String(settings.roi_height) + R"rawliteral(</p>
    </div>

    <script>
        async function saveSettings() {
            const formData = new FormData();
            formData.append('width', document.getElementById('width').value);
            formData.append('height', document.getElementById('height').value);
            formData.append('x', document.getElementById('x').value);
            formData.append('y', document.getElementById('y').value);
            
            try {
                const response = await fetch('/save_roi', { method: 'POST', body: formData });
                if (response.ok) {
                    alert('ROI settings saved successfully!');
                } else {
                    alert('Error saving ROI settings!');
                }
            } catch (error) {
                alert('Error saving ROI settings: ' + error);
            }
        }
    </script>
</body>
</html>
)rawliteral";
    return html;
}