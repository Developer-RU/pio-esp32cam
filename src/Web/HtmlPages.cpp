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
 * @brief Общий шаблон с сайдбаром
 */
String getBaseTemplate(const String& title, const String& content)
{
    String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>Car Detector - )rawliteral";
    
    html += title;
    html += R"rawliteral(</title>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <meta charset="UTF-8">
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
            font-family: 'Segoe UI', Arial, sans-serif;
        }
        
        body {
            display: flex;
            min-height: 100vh;
            background: #f5f7fa;
            color: #333;
        }
        
        /* Сайдбар */
        #sidebar {
            width: 250px;
            background: #2c3e50;
            color: white;
            position: fixed;
            height: 100vh;
            overflow-y: auto;
            transition: transform 0.3s ease;
            z-index: 1000;
            box-shadow: 2px 0 10px rgba(0,0,0,0.1);
        }
        
        .sidebar-header {
            padding: 20px;
            background: #1a252f;
            text-align: center;
        }
        
        .sidebar-header h3 {
            color: white;
            font-size: 1.5rem;
        }
        
        .sidebar-menu {
            padding: 20px 0;
        }
        
        .sidebar-menu a {
            display: block;
            padding: 12px 20px;
            color: #bdc3c7;
            text-decoration: none;
            border-left: 3px solid transparent;
            transition: all 0.3s;
        }
        
        .sidebar-menu a:hover {
            background: #34495e;
            color: white;
            border-left: 3px solid #3498db;
        }
        
        .sidebar-menu a.active {
            background: #34495e;
            color: white;
            border-left: 3px solid #3498db;
        }
        
        /* Кнопка меню */
        #menu-toggle {
            position: fixed;
            top: 15px;
            left: 15px;
            background: #3498db;
            color: white;
            border: none;
            width: 40px;
            height: 40px;
            border-radius: 5px;
            cursor: pointer;
            z-index: 1001;
            display: none;
            font-size: 20px;
        }
        
        /* Основной контент */
        #content {
            flex: 1;
            margin-left: 250px;
            padding: 20px;
            transition: margin-left 0.3s ease;
        }
        
        .content-header {
            background: white;
            padding: 20px;
            border-radius: 10px;
            box-shadow: 0 2px 10px rgba(0,0,0,0.05);
            margin-bottom: 20px;
            display: flex;
            justify-content: space-between;
            align-items: center;
        }
        
        .page-title {
            font-size: 1.8rem;
            color: #2c3e50;
        }
        
        .status-badge {
            padding: 8px 15px;
            border-radius: 20px;
            font-size: 0.9rem;
            font-weight: 600;
        }
        
        .status-online {
            background: #d4edda;
            color: #155724;
        }
        
        .status-offline {
            background: #f8d7da;
            color: #721c24;
        }
        
        /* Карточки */
        .card {
            background: white;
            border-radius: 10px;
            padding: 25px;
            margin-bottom: 20px;
            box-shadow: 0 2px 10px rgba(0,0,0,0.05);
            border: 1px solid #eee;
        }
        
        .card-title {
            font-size: 1.3rem;
            color: #2c3e50;
            margin-bottom: 20px;
            padding-bottom: 10px;
            border-bottom: 2px solid #3498db;
        }
        
        /* Формы */
        .form-group {
            margin-bottom: 20px;
        }
        
        .form-label {
            display: block;
            margin-bottom: 8px;
            font-weight: 600;
            color: #2c3e50;
        }
        
        .form-control {
            width: 100%;
            padding: 12px;
            border: 1px solid #ddd;
            border-radius: 6px;
            font-size: 1rem;
            transition: border-color 0.3s;
        }
        
        .form-control:focus {
            outline: none;
            border-color: #3498db;
            box-shadow: 0 0 0 2px rgba(52, 152, 219, 0.2);
        }
        
        /* Слайдеры */
        .slider-container {
            margin-bottom: 20px;
        }
        
        .slider-value {
            display: flex;
            justify-content: space-between;
            margin-bottom: 10px;
        }
        
        .value-display {
            background: #3498db;
            color: white;
            padding: 5px 15px;
            border-radius: 15px;
            font-weight: 600;
        }
        
        input[type="range"] {
            width: 100%;
            height: 8px;
            border-radius: 4px;
            background: #ddd;
            outline: none;
        }
        
        /* Кнопки */
        .btn {
            display: inline-block;
            padding: 12px 25px;
            background: #3498db;
            color: white;
            border: none;
            border-radius: 6px;
            font-size: 1rem;
            font-weight: 600;
            cursor: pointer;
            text-decoration: none;
            transition: background 0.3s;
            text-align: center;
        }
        
        .btn:hover {
            background: #2980b9;
        }
        
        .btn-block {
            display: block;
            width: 100%;
        }
        
        .btn-success {
            background: #2ecc71;
        }
        
        .btn-success:hover {
            background: #27ae60;
        }
        
        /* Статус иконки */
        .status-item {
            display: flex;
            align-items: center;
            margin-bottom: 15px;
            padding: 15px;
            background: #f8f9fa;
            border-radius: 8px;
            border-left: 4px solid #3498db;
        }
        
        .status-icon {
            width: 50px;
            height: 50px;
            border-radius: 50%;
            background: #3498db;
            color: white;
            display: flex;
            align-items: center;
            justify-content: center;
            margin-right: 15px;
            font-size: 20px;
        }
        
        /* Адаптивность */
        @media (max-width: 768px) {
            #sidebar {
                transform: translateX(-100%);
            }
            
            #sidebar.active {
                transform: translateX(0);
            }
            
            #content {
                margin-left: 0;
            }
            
            #menu-toggle {
                display: flex;
                align-items: center;
                justify-content: center;
            }
            
            .content-header {
                flex-direction: column;
                gap: 15px;
                text-align: center;
            }
        }
        
        /* Уведомления */
        .notification {
            position: fixed;
            top: 20px;
            right: 20px;
            padding: 15px 25px;
            border-radius: 6px;
            color: white;
            font-weight: 600;
            z-index: 9999;
            animation: slideIn 0.3s ease;
        }
        
        .notification.success {
            background: #2ecc71;
        }
        
        .notification.error {
            background: #e74c3c;
        }
        
        @keyframes slideIn {
            from {
                transform: translateX(100%);
                opacity: 0;
            }
            to {
                transform: translateX(0);
                opacity: 1;
            }
        }
    </style>
</head>
<body>
    <!-- Кнопка меню для мобильных -->
    <button id="menu-toggle" onclick="toggleSidebar()">☰</button>
    
    <!-- Сайдбар -->
    <div id="sidebar">
        <div class="sidebar-header">
            <h3>Car Detector</h3>
        </div>
        <div class="sidebar-menu">
            <a href="/" class="active">Dashboard</a>
            <a href="/detection">Detection</a>
            <a href="/wifi">WiFi</a>
            <a href="/roi">ROI</a>
            <a href="/list_photos">Photos</a>
        </div>
        <div style="padding: 20px; border-top: 1px solid #34495e; margin-top: 20px;">
            <div style="color: #bdc3c7; font-size: 0.9rem; margin-bottom: 10px;">System Status</div>
            <div class="status-badge )rawliteral";
    
    html += (camera_initialized && sd_initialized ? "status-online" : "status-offline");
    html += R"rawliteral(">
                )rawliteral";
    html += (camera_initialized && sd_initialized ? "System Online" : "System Warning");
    html += R"rawliteral(
            </div>
        </div>
    </div>
    
    <!-- Основной контент -->
    <div id="content">
        <div class="content-header">
            <h1 class="page-title">)rawliteral";
    html += title;
    html += R"rawliteral(</h1>
            <div style="display: flex; gap: 10px; align-items: center;">
                <span style="color: #7f8c8d;">IP: )rawliteral";
    html += WiFi.softAPIP().toString();
    html += R"rawliteral(</span>
                <span class="status-badge status-online">
                    )rawliteral";
    html += String(WiFi.softAPgetStationNum());
    html += R"rawliteral( connected
                </span>
            </div>
        </div>
        
        )rawliteral";
    html += content;
    html += R"rawliteral(
    </div>

    <script>
        function toggleSidebar() {
            document.getElementById('sidebar').classList.toggle('active');
            document.getElementById('content').style.marginLeft = 
                document.getElementById('sidebar').classList.contains('active') ? '250px' : '0';
        }
        
        // Закрытие сайдбара при клике вне его на мобильных
        document.addEventListener('click', function(event) {
            const sidebar = document.getElementById('sidebar');
            const menuToggle = document.getElementById('menu-toggle');
            const content = document.getElementById('content');
            
            if (window.innerWidth <= 768 && 
                !sidebar.contains(event.target) && 
                !menuToggle.contains(event.target) &&
                sidebar.classList.contains('active')) {
                sidebar.classList.remove('active');
                content.style.marginLeft = '0';
            }
        });
        
        // Обновление значений слайдеров
        document.querySelectorAll('input[type="range"]').forEach(slider => {
            const valueId = slider.id + 'Value';
            const valueDisplay = document.getElementById(valueId);
            if (valueDisplay) {
                slider.addEventListener('input', function() {
                    valueDisplay.textContent = this.value;
                });
            }
        });
        
        // Показ уведомлений
        function showNotification(message, type) {
            const notification = document.createElement('div');
            notification.className = 'notification ' + type;
            notification.textContent = message;
            document.body.appendChild(notification);
            
            setTimeout(() => {
                notification.remove();
            }, 3000);
        }
        
        // Показ/скрытие пароля
        function togglePassword() {
            const passwordInput = document.getElementById('password');
            const toggleIcon = document.getElementById('togglePassword');
            if (passwordInput.type === 'password') {
                passwordInput.type = 'text';
                toggleIcon.textContent = '👁️';
            } else {
                passwordInput.type = 'password';
                toggleIcon.textContent = '👁️‍🗨️';
            }
        }
    </script>
</body>
</html>
)rawliteral";
    return html;
}

/**
 * @brief Генерация HTML главной страницы
 */
String getMainPage()
{
    String content = R"rawliteral(
        <div class="card">
            <h2 class="card-title">System Status</h2>
            
            <div class="status-item">
                <div class="status-icon">
                    📷
                </div>
                <div>
                    <h3 style="margin-bottom: 5px;">Camera</h3>
                    <p style="color: )rawliteral";
    content += (camera_initialized ? "#27ae60" : "#e74c3c");
    content += R"rawliteral(;">
                        )rawliteral";
    content += (camera_initialized ? "✓ Operational" : "✗ Not Initialized");
    content += R"rawliteral(
                    </p>
                </div>
            </div>
            
            <div class="status-item">
                <div class="status-icon">
                    💾
                </div>
                <div>
                    <h3 style="margin-bottom: 5px;">SD Card</h3>
                    <p style="color: )rawliteral";
    content += (sd_initialized ? "#27ae60" : "#e74c3c");
    content += R"rawliteral(;">
                        )rawliteral";
    content += (sd_initialized ? "✓ Mounted" : "✗ Not Found");
    content += R"rawliteral(
                    </p>
                </div>
            </div>
            
            <div class="status-item">
                <div class="status-icon">
                    📶
                </div>
                <div>
                    <h3 style="margin-bottom: 5px;">WiFi Access Point</h3>
                    <p>Clients: )rawliteral";
    content += String(WiFi.softAPgetStationNum());
    content += R"rawliteral(</p>
                </div>
            </div>
        </div>
        
        <div class="card">
            <h2 class="card-title">Quick Actions</h2>
            <div style="display: grid; gap: 10px;">
                <a href="/detection" class="btn">
                    Detection Settings
                </a>
                <a href="/wifi" class="btn">
                    WiFi Settings
                </a>
                <a href="/roi" class="btn">
                    ROI Settings
                </a>
                <a href="/list_photos" class="btn">
                    View Photos
                </a>
            </div>
        </div>
        
        <div class="card">
            <h2 class="card-title">Current Settings</h2>
            <div style="display: grid; grid-template-columns: repeat(auto-fill, minmax(200px, 1fr)); gap: 15px;">
                <div>
                    <h4>Detection Interval</h4>
                    <p>)rawliteral";
    content += String(settings.interval);
    content += R"rawliteral( ms</p>
                </div>
                <div>
                    <h4>Distance Threshold</h4>
                    <p>)rawliteral";
    content += String(settings.distance);
    content += R"rawliteral(</p>
                </div>
                <div>
                    <h4>Max Files</h4>
                    <p>)rawliteral";
    content += String(settings.max_files);
    content += R"rawliteral(</p>
                </div>
                <div>
                    <h4>ROI Size</h4>
                    <p>)rawliteral";
    content += String(settings.roi_width);
    content += R"rawliteral( x )rawliteral";
    content += String(settings.roi_height);
    content += R"rawliteral(</p>
                </div>
            </div>
        </div>
    )rawliteral";
    
    return getBaseTemplate("Dashboard", content);
}

/**
 * @brief Генерация HTML страницы настроек детекции
 */
String getDetectionSettingsPage()
{
    String content = R"rawliteral(
        <div class="card">
            <h2 class="card-title">Detection Settings</h2>
            
            <form id="detectionForm">
                <div class="slider-container">
                    <div class="slider-value">
                        <label class="form-label">Distance Threshold</label>
                        <span class="value-display" id="distanceValue">)rawliteral";
    content += String(settings.distance);
    content += R"rawliteral(</span>
                    </div>
                    <input type="range" id="distance" min="0" max="400" value=")rawliteral";
    content += String(settings.distance);
    content += R"rawliteral(">
                </div>
                
                <div class="form-group">
                    <label class="form-label">Detection Interval (ms)</label>
                    <input type="number" class="form-control" id="interval" 
                           value=")rawliteral";
    content += String(settings.interval);
    content += R"rawliteral(" min="0" max="10000" step="100">
                </div>
                
                <div class="slider-container">
                    <div class="slider-value">
                        <label class="form-label">Pixel Threshold</label>
                        <span class="value-display" id="thresholdValue">)rawliteral";
    content += String(settings.threshold);
    content += R"rawliteral(</span>
                    </div>
                    <input type="range" id="threshold" min="0" max="255" value=")rawliteral";
    content += String(settings.threshold);
    content += R"rawliteral(">
                </div>
                
                <div class="form-group">
                    <label class="form-label">Minimum Area</label>
                    <input type="number" class="form-control" id="area" 
                           value=")rawliteral";
    content += String(settings.area);
    content += R"rawliteral(" min="1" max="10000">
                </div>
                
                <div style="display: grid; grid-template-columns: 1fr 1fr; gap: 15px;">
                    <div class="form-group">
                        <label class="form-label">Dark Pixel Min</label>
                        <input type="number" class="form-control" id="dark_min" step="0.01"
                               value=")rawliteral";
    content += String(settings.dark_min);
    content += R"rawliteral(" min="0" max="1">
                    </div>
                    <div class="form-group">
                        <label class="form-label">Dark Pixel Max</label>
                        <input type="number" class="form-control" id="dark_max" step="0.01"
                               value=")rawliteral";
    content += String(settings.dark_max);
    content += R"rawliteral(" min="0" max="1">
                    </div>
                </div>
                
                <div class="form-group">
                    <label class="form-label">Texture Sensitivity</label>
                    <input type="number" class="form-control" id="texture" 
                           value=")rawliteral";
    content += String(settings.texture);
    content += R"rawliteral(" min="0" max="10000">
                </div>
                
                <div class="form-group">
                    <label class="form-label">Maximum Files</label>
                    <input type="number" class="form-control" id="max_files" 
                           value=")rawliteral";
    content += String(settings.max_files);
    content += R"rawliteral(" min="1" max="1000">
                </div>
                
                <button type="button" class="btn btn-block" onclick="saveDetectionSettings()">
                    Save Settings
                </button>
            </form>
        </div>
        
        <script>
            async function saveDetectionSettings() {
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
                        showNotification('Settings saved successfully!', 'success');
                    } else {
                        showNotification('Error saving settings!', 'error');
                    }
                } catch (error) {
                    showNotification('Error: ' + error, 'error');
                }
            }
        </script>
    )rawliteral";
    
    return getBaseTemplate("Detection Settings", content);
}

/**
 * @brief Генерация HTML страницы настроек Wi-Fi
 */
String getWifiSettingsPage()
{
    String content = R"rawliteral(
        <div class="card">
            <h2 class="card-title">WiFi Settings</h2>
            
            <div style="background: #e8f4fc; padding: 20px; border-radius: 8px; margin-bottom: 20px;">
                <h3 style="color: #3498db; margin-bottom: 10px;">Current Configuration</h3>
                <p><strong>SSID:</strong> )rawliteral";
    content += settings.ap_ssid;
    content += R"rawliteral(</p>
                <p><strong>Password:</strong> ••••••••</p>
                <p><strong>IP Address:</strong> )rawliteral";
    content += WiFi.softAPIP().toString();
    content += R"rawliteral(</p>
            </div>
            
            <form id="wifiForm">
                <div class="form-group">
                    <label class="form-label">New SSID</label>
                    <input type="text" class="form-control" id="ssid" 
                           value=")rawliteral";
    content += settings.ap_ssid;
    content += R"rawliteral(" required>
                </div>
                
                <div class="form-group">
                    <label class="form-label">New Password</label>
                    <div style="position: relative;">
                        <input type="password" class="form-control" id="password" 
                               value=")rawliteral";
    content += settings.ap_password;
    content += R"rawliteral(" required minlength="8">
                        <button type="button" onclick="togglePassword()" 
                                style="position: absolute; right: 10px; top: 50%; transform: translateY(-50%);
                                       background: none; border: none; cursor: pointer; font-size: 20px;"
                                id="togglePassword">
                            👁️‍🗨️
                        </button>
                    </div>
                </div>
                
                <div style="background: #fff3cd; padding: 15px; border-radius: 6px; margin-bottom: 20px;
                    border-left: 4px solid #ffc107;">
                    <p style="color: #856404; margin: 0;">
                        <strong>Note:</strong> Changes will take effect after device reboot.
                    </p>
                </div>
                
                <button type="button" class="btn btn-success btn-block" onclick="saveWifiSettings()">
                    Save & Reboot
                </button>
            </form>
        </div>
        
        <script>
            async function saveWifiSettings() {
                const ssid = document.getElementById('ssid').value;
                const password = document.getElementById('password').value;
                
                if (password.length < 8) {
                    showNotification('Password must be at least 8 characters!', 'error');
                    return;
                }
                
                if (!confirm('Device will reboot with new WiFi settings. Continue?')) {
                    return;
                }
                
                const formData = new FormData();
                formData.append('ssid', ssid);
                formData.append('password', password);
                
                try {
                    const response = await fetch('/save_wifi', { method: 'POST', body: formData });
                    if (response.ok) {
                        showNotification('WiFi settings saved! Rebooting...', 'success');
                        setTimeout(() => {
                            window.location.href = '/';
                        }, 2000);
                    } else {
                        showNotification('Error saving settings!', 'error');
                    }
                } catch (error) {
                    showNotification('Error: ' + error, 'error');
                }
            }
        </script>
    )rawliteral";
    
    return getBaseTemplate("WiFi Settings", content);
}

/**
 * @brief Генерация HTML страницы настроек ROI
 */
String getROISettingsPage()
{
    String content = R"rawliteral(
        <div class="card">
            <h2 class="card-title">ROI Settings</h2>
            
            <div style="text-align: center; margin-bottom: 20px;">
                <div style="display: inline-block; width: 200px; height: 150px; 
                    background: #ecf0f1; border: 2px solid #bdc3c7; border-radius: 8px;
                    position: relative; overflow: hidden;">
                    <div style="position: absolute; 
                        top: )rawliteral";
    content += String(settings.roi_y * 150 / 120);
    content += R"rawliteral(px;
                        left: )rawliteral";
    content += String(settings.roi_x * 200 / 160);
    content += R"rawliteral(px;
                        width: )rawliteral";
    content += String(settings.roi_width * 200 / 160);
    content += R"rawliteral(px;
                        height: )rawliteral";
    content += String(settings.roi_height * 150 / 120);
    content += R"rawliteral(px;
                        background: rgba(52, 152, 219, 0.3); border: 2px solid #3498db;">
                        <span style="position: absolute; top: 50%; left: 50%; 
                            transform: translate(-50%, -50%); color: #3498db; font-weight: bold;">
                            ROI
                        </span>
                    </div>
                </div>
                <p style="color: #7f8c8d; margin-top: 10px;">Image: 160x120 pixels</p>
            </div>
            
            <form id="roiForm">
                <div style="display: grid; grid-template-columns: 1fr 1fr; gap: 15px; margin-bottom: 15px;">
                    <div class="form-group">
                        <label class="form-label">Width (1-160)</label>
                        <input type="number" class="form-control" id="width" 
                               value=")rawliteral";
    content += String(settings.roi_width);
    content += R"rawliteral(" min="1" max="160">
                    </div>
                    <div class="form-group">
                        <label class="form-label">Height (1-120)</label>
                        <input type="number" class="form-control" id="height" 
                               value=")rawliteral";
    content += String(settings.roi_height);
    content += R"rawliteral(" min="1" max="120">
                    </div>
                </div>
                
                <div style="display: grid; grid-template-columns: 1fr 1fr; gap: 15px; margin-bottom: 20px;">
                    <div class="form-group">
                        <label class="form-label">X Offset (0-160)</label>
                        <input type="number" class="form-control" id="x" 
                               value=")rawliteral";
    content += String(settings.roi_x);
    content += R"rawliteral(" min="0" max="160">
                    </div>
                    <div class="form-group">
                        <label class="form-label">Y Offset (0-120)</label>
                        <input type="number" class="form-control" id="y" 
                               value=")rawliteral";
    content += String(settings.roi_y);
    content += R"rawliteral(" min="0" max="120">
                    </div>
                </div>
                
                <div style="background: #e8f6f3; padding: 15px; border-radius: 6px; margin-bottom: 20px;
                    border-left: 4px solid #1abc9c;">
                    <p style="color: #0d6256; margin: 0; font-size: 0.9rem;">
                        <strong>Current ROI:</strong> 
                        X=)rawliteral";
    content += String(settings.roi_x);
    content += R"rawliteral(, 
                        Y=)rawliteral";
    content += String(settings.roi_y);
    content += R"rawliteral(, 
                        W=)rawliteral";
    content += String(settings.roi_width);
    content += R"rawliteral(, 
                        H=)rawliteral";
    content += String(settings.roi_height);
    content += R"rawliteral(
                    </p>
                </div>
                
                <button type="button" class="btn btn-block" onclick="saveROISettings()">
                    Save ROI Settings
                </button>
            </form>
        </div>
        
        <script>
            async function saveROISettings() {
                const width = parseInt(document.getElementById('width').value);
                const height = parseInt(document.getElementById('height').value);
                const x = parseInt(document.getElementById('x').value);
                const y = parseInt(document.getElementById('y').value);
                
                if (x + width > 160) {
                    showNotification('ROI exceeds image width!', 'error');
                    return;
                }
                if (y + height > 120) {
                    showNotification('ROI exceeds image height!', 'error');
                    return;
                }
                
                const formData = new FormData();
                formData.append('width', width);
                formData.append('height', height);
                formData.append('x', x);
                formData.append('y', y);
                
                try {
                    const response = await fetch('/save_roi', { method: 'POST', body: formData });
                    if (response.ok) {
                        showNotification('ROI settings saved!', 'success');
                    } else {
                        showNotification('Error saving settings!', 'error');
                    }
                } catch (error) {
                    showNotification('Error: ' + error, 'error');
                }
            }
        </script>
    )rawliteral";
    
    return getBaseTemplate("ROI Settings", content);
}

// // // /**
// // //  * @brief Генерация HTML страницы со списком фото
// // //  */
// // // String getPhotosListPage() .. getPhotosListPageSimple
// // // {
// // //     String content = R"rawliteral(
// // //         <div class="card">
// // //             <h2 class="card-title">Photo Gallery</h2>
            
// // //             <div style="text-align: center; padding: 30px 20px;">
// // //                 <div style="font-size: 48px; color: #3498db; margin-bottom: 20px;">
// // //                     📷
// // //                 </div>
// // //                 <h3 style="margin-bottom: 10px;">Captured Photos</h3>
// // //                 <p style="color: #7f8c8d; margin-bottom: 30px;">
// // //                     View and manage photos stored on SD card
// // //                 </p>
                
// // //                 <div style="display: flex; gap: 15px; justify-content: center; flex-wrap: wrap;">
// // //                     <a href="/list_files" class="btn">
// // //                         List All Photos
// // //                     </a>
// // //                     <a href="/download_photo?file=latest" class="btn">
// // //                         Download Latest
// // //                     </a>
// // //                 </div>
// // //             </div>
            
// // //             <div style="background: #f8f9fa; padding: 20px; border-radius: 8px; margin-top: 20px;">
// // //                 <h4 style="color: #2c3e50; margin-bottom: 10px;">
// // //                     Information
// // //                 </h4>
// // //                 <ul style="color: #7f8c8d; padding-left: 20px;">
// // //                     <li>Photos are automatically saved when motion is detected</li>
// // //                     <li>Maximum files: )rawliteral";
// // //     content += String(settings.max_files);
// // //     content += R"rawliteral(</li>
// // //                     <li>Oldest files are automatically deleted when limit is reached</li>
// // //                     <li>Photos are stored in JPEG format on SD card</li>
// // //                 </ul>
// // //             </div>
// // //         </div>
// // //     )rawliteral";
    
// // //     return getBaseTemplate("Photos", content);
// // // }





/**
 * @brief Генерация HTML страницы галереи фотографий
 */
String getPhotosListPage()
{
    String content = R"rawliteral(
        <div class="card">
            <h2 class="card-title">Photo Gallery</h2>
            
            <div style="margin-bottom: 20px;">
                <button class="btn" onclick="location.reload()" style="margin-right: 10px;">
                    <i class="fas fa-sync-alt"></i> Refresh
                </button>
                <button class="btn" onclick="location.href='/list_files'" style="background: #f8f9fa; color: #333;">
                    <i class="fas fa-list"></i> View File List
                </button>
            </div>
            
            <!-- Галерея фотографий -->
            <div id="gallery">
                )rawliteral";
    
    if (!sd_initialized) {
        content += R"rawliteral(
                <div style="text-align: center; padding: 40px; color: #e74c3c;">
                    <h3>SD Card Error</h3>
                    <p>SD card not available. Check the card and restart the device.</p>
                </div>
                )rawliteral";
    } else {
        File root = SD_MMC.open("/");
        if (!root) {
            content += R"rawliteral(
                <div style="text-align: center; padding: 40px; color: #e74c3c;">
                    <h3>SD Card Error</h3>
                    <p>Failed to open SD card root directory.</p>
                </div>
                )rawliteral";
        } else {
            // Счетчик файлов
            int fileCount = 0;
            std::vector<String> jpgFiles;
            
            // Сначала собираем все JPG файлы
            File file = root.openNextFile();
            while (file) {
                String fileName = file.name();
                if (!file.isDirectory() && 
                    (fileName.endsWith(".jpg") || fileName.endsWith(".JPG"))) {
                    jpgFiles.push_back(fileName);
                    fileCount++;
                }
                file.close();
                file = root.openNextFile();
            }
            root.close();
            
            if (fileCount == 0) {
                content += R"rawliteral(
                    <div style="text-align: center; padding: 40px; color: #7f8c8d;">
                        <h3>No Photos Found</h3>
                        <p>No photos have been captured yet.</p>
                    </div>
                    )rawliteral";
            } else {
                // Сортируем файлы по имени (новые сначала)
                std::sort(jpgFiles.begin(), jpgFiles.end(), [](const String& a, const String& b) {
                    return a > b;
                });
                
                // Ограничиваем количество отображаемых файлов
                int maxFiles = 30; // Не более 30 фотографий для скорости загрузки
                if (jpgFiles.size() > maxFiles) {
                    jpgFiles.resize(maxFiles);
                }
                
                // Создаем сетку карточек
                content += "<div style=\"display: grid; grid-template-columns: repeat(auto-fill, minmax(250px, 1fr)); gap: 20px;\">";
                
                for (const String& fileName : jpgFiles) {
                    // Получаем размер файла для отображения
                    File imgFile = SD_MMC.open(("/" + fileName).c_str(), FILE_READ);
                    size_t fileSize = 0;
                    if (imgFile) {
                        fileSize = imgFile.size();
                        imgFile.close();
                    }
                    
                    // Форматируем размер
                    String sizeStr;
                    if (fileSize < 1024) {
                        sizeStr = String(fileSize) + " B";
                    } else if (fileSize < 1024 * 1024) {
                        sizeStr = String(fileSize / 1024.0, 1) + " KB";
                    } else {
                        sizeStr = String(fileSize / (1024.0 * 1024.0), 1) + " MB";
                    }
                    
                    content += "<div style=\"background: white; border-radius: 8px; overflow: hidden; box-shadow: 0 2px 8px rgba(0,0,0,0.1);\">";
                    
                    // Изображение с ссылкой на скачивание
                    content += "<a href=\"/download_photo?file=" + fileName + "\" style=\"display: block;\">";
                    content += "<div style=\"width: 100%; height: 180px; overflow: hidden;\">";
                    content += "<img src=\"/download_photo?file=" + fileName + "\" alt=\"" + fileName + "\" ";
                    content += "style=\"width: 100%; height: 100%; object-fit: cover; display: block;\">";
                    content += "</div>";
                    content += "</a>";
                    
                    // Информация о файле
                    content += "<div style=\"padding: 12px;\">";
                    
                    // Имя файла (обрезаем если слишком длинное)
                    String displayName = fileName;
                    if (displayName.length() > 24) {
                        displayName = displayName.substring(0, 21) + "...";
                    }
                    content += "<div style=\"font-weight: 600; color: #2c3e50; margin-bottom: 5px; font-size: 14px;\">" + displayName + "</div>";
                    
                    // Размер файла
                    content += "<div style=\"font-size: 12px; color: #7f8c8d; display: flex; justify-content: space-between;\">";
                    content += "<span>Size: " + sizeStr + "</span>";
                    
                    // Кнопка удаления
                    content += "<button onclick=\"deletePhoto('" + fileName + "')\" ";
                    content += "style=\"background: none; border: none; color: #e74c3c; cursor: pointer; font-size: 12px; padding: 0;\">";
                    content += "<i class=\"fas fa-trash\"></i> Delete</button>";
                    
                    content += "</div>";
                    content += "</div>";
                    
                    content += "</div>";
                }
                
                content += "</div>";
                
                // Показываем количество файлов
                content += "<div style=\"margin-top: 20px; padding: 10px; background: #f8f9fa; border-radius: 6px; text-align: center;\">";
                content += "<span style=\"color: #7f8c8d; font-size: 14px;\">Showing " + String(fileCount) + " photos</span>";
                content += "</div>";
            }
        }
    }
    
    content += R"rawliteral(
            </div>
        </div>
        
        <script>
            function deletePhoto(filename) {
                if (!confirm('Delete ' + filename + '?')) {
                    return;
                }
                
                fetch('/delete_photo?file=' + encodeURIComponent(filename), {
                    method: 'POST'
                })
                .then(response => {
                    if (response.ok) {
                        alert('Photo deleted');
                        location.reload();
                    } else {
                        alert('Error deleting photo');
                    }
                })
                .catch(error => {
                    alert('Error: ' + error);
                });
            }
            
            // Предзагрузка изображений при наведении для лучшего UX
            document.addEventListener('DOMContentLoaded', function() {
                const images = document.querySelectorAll('#gallery img');
                images.forEach(img => {
                    // Добавляем эффект при наведении
                    img.parentElement.parentElement.addEventListener('mouseenter', function() {
                        this.style.transform = 'translateY(-4px)';
                        this.style.boxShadow = '0 6px 12px rgba(0,0,0,0.15)';
                    });
                    
                    img.parentElement.parentElement.addEventListener('mouseleave', function() {
                        this.style.transform = 'translateY(0)';
                        this.style.boxShadow = '0 2px 8px rgba(0,0,0,0.1)';
                    });
                });
            });
        </script>
        
        <style>
            #gallery > div > div {
                transition: all 0.2s ease;
            }
            
            #gallery img {
                transition: transform 0.3s ease;
            }
            
            #gallery a:hover img {
                transform: scale(1.05);
            }
            
            @media (max-width: 768px) {
                #gallery > div {
                    grid-template-columns: repeat(auto-fill, minmax(200px, 1fr));
                    gap: 15px;
                }
            }
        </style>
    )rawliteral";
    
    return getBaseTemplate("Photo Gallery", content);
}