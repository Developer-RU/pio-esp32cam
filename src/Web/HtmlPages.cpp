/**
 * @file HtmlPages.cpp
 * @brief Реализация генерации HTML страниц с боковым меню
 */

#include "Web/HtmlPages.hpp"
#include "Config/Config.hpp"
#include <WiFi.h>

// Внешние объявления
extern bool camera_initialized;
extern bool sd_initialized;
extern Settings settings;

// Общие функции для генерации HTML
String getHeader(const String& title) {
    String html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>)rawliteral";
    html += title;
    html += R"rawliteral( - Car Detector</title>
    <style>
        :root {
            --primary-color: #4361ee;
            --secondary-color: #3a0ca3;
            --success-color: #4cc9f0;
            --warning-color: #f72585;
            --dark-color: #1a1a2e;
            --light-color: #f8f9fa;
            --sidebar-width: 250px;
            --border-radius: 8px;
        }
        
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            color: #333;
            display: flex;
        }
        
        /* Сайдбар */
        .sidebar {
            width: var(--sidebar-width);
            background-color: rgba(26, 26, 46, 0.95);
            backdrop-filter: blur(10px);
            padding: 20px 0;
            height: 100vh;
            position: fixed;
            left: 0;
            top: 0;
            display: flex;
            flex-direction: column;
            box-shadow: 5px 0 15px rgba(0, 0, 0, 0.1);
            z-index: 1000;
        }
        
        .sidebar-header {
            padding: 0 20px 20px;
            border-bottom: 1px solid rgba(255, 255, 255, 0.1);
            margin-bottom: 20px;
        }
        
        .sidebar-header h2 {
            color: white;
            font-size: 1.2rem;
            display: flex;
            align-items: center;
            gap: 10px;
        }
        
        .sidebar-header h2 i {
            color: var(--success-color);
        }
        
        .nav-menu {
            flex: 1;
            overflow-y: auto;
        }
        
        .nav-item {
            padding: 12px 25px;
            color: rgba(255, 255, 255, 0.8);
            text-decoration: none;
            display: flex;
            align-items: center;
            gap: 12px;
            transition: all 0.3s ease;
            border-left: 3px solid transparent;
        }
        
        .nav-item:hover {
            background-color: rgba(67, 97, 238, 0.2);
            color: white;
            border-left-color: var(--primary-color);
        }
        
        .nav-item.active {
            background-color: rgba(67, 97, 238, 0.3);
            color: white;
            border-left-color: var(--primary-color);
        }
        
        .nav-item i {
            width: 20px;
            text-align: center;
        }
        
        .system-status {
            padding: 15px 25px;
            background-color: rgba(0, 0, 0, 0.2);
            margin: 10px 20px;
            border-radius: var(--border-radius);
            border: 1px solid rgba(255, 255, 255, 0.1);
        }
        
        .status-item {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin: 8px 0;
            font-size: 0.9rem;
        }
        
        .status-label {
            color: rgba(255, 255, 255, 0.7);
        }
        
        .status-value {
            font-weight: 600;
            padding: 3px 8px;
            border-radius: 4px;
            font-size: 0.85rem;
        }
        
        .status-ok {
            background-color: rgba(76, 201, 240, 0.2);
            color: #4cc9f0;
        }
        
        .status-error {
            background-color: rgba(247, 37, 133, 0.2);
            color: #f72585;
        }
        
        /* Основной контент */
        .main-content {
            flex: 1;
            margin-left: var(--sidebar-width);
            padding: 30px;
            max-width: calc(100vw - var(--sidebar-width));
            overflow-x: hidden;
        }
        
        .page-header {
            background: white;
            padding: 25px;
            border-radius: var(--border-radius);
            box-shadow: 0 5px 15px rgba(0, 0, 0, 0.1);
            margin-bottom: 30px;
            display: flex;
            justify-content: space-between;
            align-items: center;
            flex-wrap: wrap;
        }
        
        .page-title {
            color: var(--dark-color);
            font-size: 1.8rem;
        }
        
        .page-subtitle {
            color: #666;
            margin-top: 5px;
            font-size: 1rem;
        }
        
        .current-ip {
            background: linear-gradient(135deg, var(--primary-color), var(--secondary-color));
            color: white;
            padding: 10px 20px;
            border-radius: var(--border-radius);
            font-weight: 600;
        }
        
        /* Карточки */
        .card-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(350px, 1fr));
            gap: 25px;
            margin-bottom: 30px;
        }
        
        .card {
            background: white;
            border-radius: var(--border-radius);
            box-shadow: 0 5px 15px rgba(0, 0, 0, 0.1);
            overflow: hidden;
            transition: transform 0.3s ease, box-shadow 0.3s ease;
        }
        
        .card:hover {
            transform: translateY(-5px);
            box-shadow: 0 10px 25px rgba(0, 0, 0, 0.15);
        }
        
        .card-header {
            padding: 20px;
            background: linear-gradient(135deg, var(--primary-color), var(--secondary-color));
            color: white;
            border-bottom: 1px solid rgba(255, 255, 255, 0.1);
        }
        
        .card-body {
            padding: 25px;
        }
        
        /* Формы */
        .form-group {
            margin-bottom: 20px;
        }
        
        .form-label {
            display: block;
            margin-bottom: 8px;
            font-weight: 600;
            color: #444;
        }
        
        .form-control {
            width: 100%;
            padding: 12px 15px;
            border: 2px solid #e1e5e9;
            border-radius: var(--border-radius);
            font-size: 1rem;
            transition: border-color 0.3s ease;
            background-color: #f8f9fa;
        }
        
        .form-control:focus {
            outline: none;
            border-color: var(--primary-color);
            background-color: white;
        }
        
        .slider-container {
            display: flex;
            align-items: center;
            gap: 15px;
            margin-top: 5px;
        }
        
        .slider {
            flex: 1;
            height: 6px;
            -webkit-appearance: none;
            background: linear-gradient(to right, var(--primary-color), var(--secondary-color));
            border-radius: 3px;
            outline: none;
        }
        
        .slider::-webkit-slider-thumb {
            -webkit-appearance: none;
            width: 20px;
            height: 20px;
            background: white;
            border-radius: 50%;
            cursor: pointer;
            border: 2px solid var(--primary-color);
            box-shadow: 0 2px 5px rgba(0, 0, 0, 0.2);
        }
        
        .value-display {
            min-width: 40px;
            text-align: center;
            font-weight: 600;
            color: var(--primary-color);
            background-color: rgba(67, 97, 238, 0.1);
            padding: 5px 10px;
            border-radius: 4px;
        }
        
        /* Кнопки */
        .btn {
            display: inline-block;
            padding: 12px 25px;
            background: linear-gradient(135deg, var(--primary-color), var(--secondary-color));
            color: white;
            border: none;
            border-radius: var(--border-radius);
            font-size: 1rem;
            font-weight: 600;
            cursor: pointer;
            transition: all 0.3s ease;
            text-decoration: none;
            text-align: center;
        }
        
        .btn:hover {
            transform: translateY(-2px);
            box-shadow: 0 5px 15px rgba(67, 97, 238, 0.4);
        }
        
        .btn-wide {
            width: 100%;
            margin-top: 10px;
        }
        
        /* Адаптивность */
        @media (max-width: 768px) {
            .sidebar {
                width: 70px;
                padding: 15px 0;
            }
            
            .sidebar-header h2 span,
            .nav-item span,
            .status-label,
            .status-value,
            .system-status {
                display: none;
            }
            
            .nav-item {
                justify-content: center;
                padding: 15px;
            }
            
            .nav-item i {
                font-size: 1.2rem;
            }
            
            .main-content {
                margin-left: 70px;
                padding: 15px;
            }
            
            .page-header {
                padding: 15px;
            }
            
            .card-grid {
                grid-template-columns: 1fr;
            }
        }
        
        /* Иконки */
        .icon {
            display: inline-block;
            width: 24px;
            text-align: center;
            margin-right: 8px;
        }
        
        /* Вспомогательные классы */
        .text-center { text-align: center; }
        .mb-3 { margin-bottom: 1rem; }
        .mb-4 { margin-bottom: 1.5rem; }
        .mt-3 { margin-top: 1rem; }
        .mt-4 { margin-top: 1.5rem; }
        .p-3 { padding: 1rem; }
    </style>
</head>
<body>
    )rawliteral";
    
    return html;
}

String getSidebar(const String& activePage = "dashboard") {
    String sidebar = R"rawliteral(
    <div class="sidebar">
        <div class="sidebar-header">
            <h2><i class="icon">🚗</i> <span>Car Detector</span></h2>
        </div>
        
        <div class="nav-menu">
            <a href="/" class="nav-item )rawliteral";
    sidebar += (activePage == "dashboard" ? "active" : "");
    sidebar += R"rawliteral(">
                <i>📊</i> <span>Dashboard</span>
            </a>
            <a href="/detection" class="nav-item )rawliteral";
    sidebar += (activePage == "detection" ? "active" : "");
    sidebar += R"rawliteral(">
                <i>⚙️</i> <span>Detection Settings</span>
            </a>
            <a href="/wifi" class="nav-item )rawliteral";
    sidebar += (activePage == "wifi" ? "active" : "");
    sidebar += R"rawliteral(">
                <i>📶</i> <span>WiFi Settings</span>
            </a>
            <a href="/roi" class="nav-item )rawliteral";
    sidebar += (activePage == "roi" ? "active" : "");
    sidebar += R"rawliteral(">
                <i>🎯</i> <span>ROI Settings</span>
            </a>
            <a href="/list_photos" class="nav-item )rawliteral";
    sidebar += (activePage == "photos" ? "active" : "");
    sidebar += R"rawliteral(">
                <i>🖼️</i> <span>Photos</span>
            </a>
        </div>
        
        <div class="system-status">
            <div class="status-item">
                <span class="status-label">Camera:</span>
                <span class="status-value )rawliteral";
    sidebar += (camera_initialized ? "status-ok" : "status-error");
    sidebar += R"rawliteral(">
                    )rawliteral";
    sidebar += (camera_initialized ? "OK" : "ERROR");
    sidebar += R"rawliteral(
                </span>
            </div>
            <div class="status-item">
                <span class="status-label">SD Card:</span>
                <span class="status-value )rawliteral";
    sidebar += (sd_initialized ? "status-ok" : "status-error");
    sidebar += R"rawliteral(">
                    )rawliteral";
    sidebar += (sd_initialized ? "OK" : "ERROR");
    sidebar += R"rawliteral(
                </span>
            </div>
            <div class="status-item">
                <span class="status-label">Clients:</span>
                <span class="status-value status-ok">
                    )rawliteral";
    sidebar += String(WiFi.softAPgetStationNum());
    sidebar += R"rawliteral(
                </span>
            </div>
        </div>
    </div>
    
    <div class="main-content">
    )rawliteral";
    
    return sidebar;
}

String getFooter() {
    return R"rawliteral(
    </div>
    
    <script>
        // Функция для сохранения настроек
        async function saveSettings(endpoint, formData) {
            try {
                const response = await fetch(endpoint, {
                    method: 'POST',
                    body: formData
                });
                
                if (response.ok) {
                    showNotification('Settings saved successfully!', 'success');
                } else {
                    showNotification('Error saving settings!', 'error');
                }
            } catch (error) {
                showNotification('Connection error: ' + error, 'error');
            }
        }
        
        // Уведомления
        function showNotification(message, type) {
            // Создаем элемент уведомления
            const notification = document.createElement('div');
            notification.className = `notification ${type}`;
            notification.innerHTML = `
                <span class="notification-icon">${type === 'success' ? '✓' : '✗'}</span>
                <span class="notification-text">${message}</span>
            `;
            
            // Стили для уведомления
            notification.style.cssText = `
                position: fixed;
                top: 20px;
                right: 20px;
                background: ${type === 'success' ? '#4CAF50' : '#F44336'};
                color: white;
                padding: 15px 20px;
                border-radius: 8px;
                display: flex;
                align-items: center;
                gap: 10px;
                box-shadow: 0 5px 15px rgba(0,0,0,0.2);
                z-index: 10000;
                animation: slideIn 0.3s ease;
            `;
            
            document.body.appendChild(notification);
            
            // Удаляем через 3 секунды
            setTimeout(() => {
                notification.style.animation = 'slideOut 0.3s ease';
                setTimeout(() => notification.remove(), 300);
            }, 3000);
            
            // CSS анимации
            const style = document.createElement('style');
            style.textContent = `
                @keyframes slideIn {
                    from { transform: translateX(100%); opacity: 0; }
                    to { transform: translateX(0); opacity: 1; }
                }
                @keyframes slideOut {
                    from { transform: translateX(0); opacity: 1; }
                    to { transform: translateX(100%); opacity: 0; }
                }
            `;
            document.head.appendChild(style);
        }
        
        // Обновление значений слайдеров в реальном времени
        function initSliders() {
            document.querySelectorAll('.slider').forEach(slider => {
                const displayId = slider.id + 'Value';
                const display = document.getElementById(displayId);
                if (display) {
                    slider.addEventListener('input', function() {
                        display.textContent = this.value;
                    });
                }
            });
        }
        
        // Инициализация при загрузке
        document.addEventListener('DOMContentLoaded', initSliders);
    </script>
</body>
</html>
    )rawliteral";
}

/**
 * @brief Генерация HTML главной страницы
 */
String getMainPage()
{
    String html = getHeader("Dashboard");
    html += getSidebar("dashboard");
    
    html += R"rawliteral(
        <div class="page-header">
            <div>
                <h1 class="page-title">System Dashboard</h1>
                <p class="page-subtitle">Car Detection System Status & Monitoring</p>
            </div>
            <div class="current-ip">
                IP: )rawliteral";
    html += WiFi.softAPIP().toString();
    html += R"rawliteral(
            </div>
        </div>
        
        <div class="card-grid">
            <div class="card">
                <div class="card-header">
                    <h3>System Status</h3>
                </div>
                <div class="card-body">
                    <div class="form-group">
                        <div class="status-item">
                            <span>Camera Module:</span>
                            <span class="status-value )rawliteral";
    html += (camera_initialized ? "status-ok" : "status-error");
    html += R"rawliteral(">
                                )rawliteral";
    html += (camera_initialized ? "Operational" : "Failed");
    html += R"rawliteral(
                            </span>
                        </div>
                        <div class="status-item">
                            <span>SD Storage:</span>
                            <span class="status-value )rawliteral";
    html += (sd_initialized ? "status-ok" : "status-error");
    html += R"rawliteral(">
                                )rawliteral";
    html += (sd_initialized ? "Available" : "Not Found");
    html += R"rawliteral(
                            </span>
                        </div>
                        <div class="status-item">
                            <span>WiFi Clients:</span>
                            <span class="status-value status-ok">
                                )rawliteral";
    html += String(WiFi.softAPgetStationNum());
    html += R"rawliteral( connected
                            </span>
                        </div>
                        <div class="status-item">
                            <span>Current Mode:</span>
                            <span class="status-value status-ok">
                                Monitoring
                            </span>
                        </div>
                    </div>
                </div>
            </div>
            
            <div class="card">
                <div class="card-header">
                    <h3>Quick Actions</h3>
                </div>
                <div class="card-body">
                    <button class="btn btn-wide mb-3" onclick="location.href='/detection'">
                        ⚙️ Detection Settings
                    </button>
                    <button class="btn btn-wide mb-3" onclick="location.href='/wifi'">
                        📶 WiFi Configuration
                    </button>
                    <button class="btn btn-wide mb-3" onclick="location.href='/roi'">
                        🎯 ROI Calibration
                    </button>
                    <button class="btn btn-wide" onclick="location.href='/list_photos'">
                        🖼️ View Photos
                    </button>
                </div>
            </div>
            
            <div class="card">
                <div class="card-header">
                    <h3>System Information</h3>
                </div>
                <div class="card-body">
                    <div class="form-group">
                        <label>Device Name</label>
                        <div class="form-control">ESP32-CAM Car Detector</div>
                        
                        <label>Firmware Version</label>
                        <div class="form-control">v1.0.0</div>
                        
                        <label>AP SSID</label>
                        <div class="form-control">)rawliteral";
    html += settings.ap_ssid;
    html += R"rawliteral(</div>
                        
                        <label>Detection Distance</label>
                        <div class="form-control">)rawliteral";
    html += String(settings.distance);
    html += R"rawliteral( cm</div>
                    </div>
                </div>
            </div>
        </div>
    )rawliteral";
    
    html += getFooter();
    return html;
}

/**
 * @brief Генерация HTML страницы настроек детекции
 */
String getDetectionSettingsPage()
{
    String html = getHeader("Detection Settings");
    html += getSidebar("detection");
    
    html += R"rawliteral(
        <div class="page-header">
            <div>
                <h1 class="page-title">Detection Settings</h1>
                <p class="page-subtitle">Configure car detection sensitivity parameters</p>
            </div>
        </div>
        
        <form id="detectionForm" onsubmit="event.preventDefault(); saveDetectionSettings();">
            <div class="card-grid">
                <div class="card">
                    <div class="card-header">
                        <h3>Distance & Timing</h3>
                    </div>
                    <div class="card-body">
                        <div class="form-group">
                            <label class="form-label">Detection Distance (cm)</label>
                            <div class="slider-container">
                                <input type="range" class="slider" id="distance" min="50" max="500" value=")rawliteral";
    html += String(settings.distance);
    html += R"rawliteral(" step="10">
                                <span class="value-display" id="distanceValue">)rawliteral";
    html += String(settings.distance);
    html += R"rawliteral(</span>
                            </div>
                            <small>Cars within this distance will trigger detection</small>
                        </div>
                        
                        <div class="form-group">
                            <label class="form-label">Check Interval (ms)</label>
                            <input type="number" class="form-control" id="interval" 
                                   value=")rawliteral";
    html += String(settings.interval);
    html += R"rawliteral(" 
                                   min="1000" max="30000" step="1000">
                            <small>Time between detection checks</small>
                        </div>
                    </div>
                </div>
                
                <div class="card">
                    <div class="card-header">
                        <h3>Image Processing</h3>
                    </div>
                    <div class="card-body">
                        <div class="form-group">
                            <label class="form-label">Pixel Threshold (0-255)</label>
                            <div class="slider-container">
                                <input type="range" class="slider" id="threshold" min="0" max="255" value=")rawliteral";
    html += String(settings.threshold);
    html += R"rawliteral(">
                                <span class="value-display" id="thresholdValue">)rawliteral";
    html += String(settings.threshold);
    html += R"rawliteral(</span>
                            </div>
                            <small>Dark pixel threshold for detection</small>
                        </div>
                        
                        <div class="form-group">
                            <label class="form-label">Minimum Area</label>
                            <input type="number" class="form-control" id="area" 
                                   value=")rawliteral";
    html += String(settings.area);
    html += R"rawliteral(" 
                                   min="1" max="10000">
                            <small>Minimum dark pixel area to trigger</small>
                        </div>
                    </div>
                </div>
                
                <div class="card">
                    <div class="card-header">
                        <h3>Advanced Settings</h3>
                    </div>
                    <div class="card-body">
                        <div class="form-group">
                            <label class="form-label">Dark Ratio Range</label>
                            <div style="display: grid; grid-template-columns: 1fr 1fr; gap: 15px;">
                                <div>
                                    <label style="font-size: 0.9rem; color: #666;">Min (0-1)</label>
                                    <input type="number" class="form-control" id="dark_min" step="0.01" 
                                           value=")rawliteral";
    html += String(settings.dark_min);
    html += R"rawliteral(" 
                                           min="0" max="1">
                                </div>
                                <div>
                                    <label style="font-size: 0.9rem; color: #666;">Max (0-1)</label>
                                    <input type="number" class="form-control" id="dark_max" step="0.01" 
                                           value=")rawliteral";
    html += String(settings.dark_max);
    html += R"rawliteral(" 
                                           min="0" max="1">
                                </div>
                            </div>
                        </div>
                        
                        <div class="form-group">
                            <label class="form-label">Max Files to Store</label>
                            <input type="number" class="form-control" id="max_files" 
                                   value=")rawliteral";
    html += String(settings.max_files);
    html += R"rawliteral(" 
                                   min="1" max="1000">
                            <small>Maximum number of photos to keep</small>
                        </div>
                    </div>
                </div>
            </div>
            
            <div class="card mt-4">
                <div class="card-body text-center">
                    <button type="submit" class="btn" style="padding: 15px 40px; font-size: 1.1rem;">
                        💾 Save Detection Settings
                    </button>
                    <p class="mt-3" style="color: #666;">
                        Changes will take effect immediately
                    </p>
                </div>
            </div>
        </form>
        
        <script>
            async function saveDetectionSettings() {
                const formData = new FormData();
                formData.append('distance', document.getElementById('distance').value);
                formData.append('interval', document.getElementById('interval').value);
                formData.append('threshold', document.getElementById('threshold').value);
                formData.append('area', document.getElementById('area').value);
                formData.append('dark_min', document.getElementById('dark_min').value);
                formData.append('dark_max', document.getElementById('dark_max').value);
                formData.append('max_files', document.getElementById('max_files').value);
                
                await saveSettings('/save_detection', formData);
            }
            
            // Initialize sliders for this page
            document.addEventListener('DOMContentLoaded', function() {
                document.getElementById('distance').oninput = function() {
                    document.getElementById('distanceValue').textContent = this.value;
                };
                
                document.getElementById('threshold').oninput = function() {
                    document.getElementById('thresholdValue').textContent = this.value;
                };
            });
        </script>
    )rawliteral";
    
    html += getFooter();
    return html;
}

/**
 * @brief Генерация HTML страницы настроек Wi-Fi
 */
String getWifiSettingsPage()
{
    String html = getHeader("WiFi Settings");
    html += getSidebar("wifi");
    
    html += R"rawliteral(
        <div class="page-header">
            <div>
                <h1 class="page-title">WiFi Access Point Settings</h1>
                <p class="page-subtitle">Configure wireless network for system access</p>
            </div>
        </div>
        
        <div class="card-grid">
            <div class="card">
                <div class="card-header">
                    <h3>Current Configuration</h3>
                </div>
                <div class="card-body">
                    <div class="form-group">
                        <label class="form-label">SSID (Network Name)</label>
                        <div class="form-control">)rawliteral";
    html += settings.ap_ssid;
    html += R"rawliteral(</div>
                        
                        <label class="form-label">Password</label>
                        <div class="form-control">••••••••</div>
                        
                        <label class="form-label">IP Address</label>
                        <div class="form-control">)rawliteral";
    html += WiFi.softAPIP().toString();
    html += R"rawliteral(</div>
                        
                        <label class="form-label">Connected Devices</label>
                        <div class="form-control">)rawliteral";
    html += String(WiFi.softAPgetStationNum());
    html += R"rawliteral( client(s)</div>
                    </div>
                </div>
            </div>
            
            <div class="card">
                <div class="card-header">
                    <h3>Update WiFi Settings</h3>
                </div>
                <div class="card-body">
                    <form id="wifiForm" onsubmit="event.preventDefault(); saveWifiSettings();">
                        <div class="form-group">
                            <label class="form-label">New SSID</label>
                            <input type="text" class="form-control" id="ssid" 
                                   value=")rawliteral";
    html += settings.ap_ssid;
    html += R"rawliteral(" 
                                   placeholder="Enter network name" required>
                        </div>
                        
                        <div class="form-group">
                            <label class="form-label">New Password</label>
                            <input type="password" class="form-control" id="password" 
                                   value=")rawliteral";
    html += settings.ap_password;
    html += R"rawliteral(" 
                                   placeholder="Minimum 8 characters" minlength="8" required>
                            <small style="color: #666;">Password must be at least 8 characters long</small>
                        </div>
                        
                        <div class="form-group">
                            <div style="background: #fff3cd; border-left: 4px solid #ffc107; padding: 15px;">
                                <strong>⚠️ Important Note:</strong>
                                <p style="margin-top: 5px; color: #856404;">
                                    Changes will take effect after system reboot. You will need to reconnect to the new network.
                                </p>
                            </div>
                        </div>
                        
                        <button type="submit" class="btn btn-wide">
                            🔄 Update WiFi Settings
                        </button>
                    </form>
                </div>
            </div>
        </div>
        
        <script>
            async function saveWifiSettings() {
                const ssid = document.getElementById('ssid').value;
                const password = document.getElementById('password').value;
                
                if (password.length < 8) {
                    showNotification('Password must be at least 8 characters!', 'error');
                    return;
                }
                
                const formData = new FormData();
                formData.append('ssid', ssid);
                formData.append('password', password);
                
                await saveSettings('/save_wifi', formData);
            }
        </script>
    )rawliteral";
    
    html += getFooter();
    return html;
}

/**
 * @brief Генерация HTML страницы настроек ROI
 */
String getROISettingsPage()
{
    String html = getHeader("ROI Settings");
    html += getSidebar("roi");
    
    html += R"rawliteral(
        <div class="page-header">
            <div>
                <h1 class="page-title">Region of Interest (ROI)</h1>
                <p class="page-subtitle">Define the detection area in the camera frame</p>
            </div>
        </div>
        
        <div class="card-grid">
            <div class="card">
                <div class="card-header">
                    <h3>Current ROI Configuration</h3>
                </div>
                <div class="card-body">
                    <div class="form-group">
                        <div style="background: #e7f3ff; padding: 20px; border-radius: var(--border-radius);">
                            <div style="display: grid; grid-template-columns: repeat(2, 1fr); gap: 15px;">
                                <div>
                                    <label style="font-size: 0.9rem; color: #666;">X Position</label>
                                    <div class="form-control">)rawliteral";
    html += String(settings.roi_x);
    html += R"rawliteral( px</div>
                                </div>
                                <div>
                                    <label style="font-size: 0.9rem; color: #666;">Y Position</label>
                                    <div class="form-control">)rawliteral";
    html += String(settings.roi_y);
    html += R"rawliteral( px</div>
                                </div>
                                <div>
                                    <label style="font-size: 0.9rem; color: #666;">Width</label>
                                    <div class="form-control">)rawliteral";
    html += String(settings.roi_width);
    html += R"rawliteral( px</div>
                                </div>
                                <div>
                                    <label style="font-size: 0.9rem; color: #666;">Height</label>
                                    <div class="form-control">)rawliteral";
    html += String(settings.roi_height);
    html += R"rawliteral( px</div>
                                </div>
                            </div>
                            
                            <div style="margin-top: 20px; padding-top: 15px; border-top: 1px solid #cfe2ff;">
                                <strong>Frame Information:</strong>
                                <p style="margin-top: 5px; color: #666; font-size: 0.9rem;">
                                    Detection frame: 160×120 pixels<br>
                                    ROI covers )rawliteral";
    html += String((settings.roi_width * settings.roi_height * 100) / (160 * 120));
    html += R"rawliteral(% of the frame
                                </p>
                            </div>
                        </div>
                    </div>
                </div>
            </div>
            
            <div class="card">
                <div class="card-header">
                    <h3>Adjust ROI Settings</h3>
                </div>
                <div class="card-body">
                    <form id="roiForm" onsubmit="event.preventDefault(); saveRoiSettings();">
                        <div class="form-group">
                            <label class="form-label">ROI Width (px)</label>
                            <input type="number" class="form-control" id="width" 
                                   value=")rawliteral";
    html += String(settings.roi_width);
    html += R"rawliteral(" 
                                   min="10" max="160">
                            <small>Maximum: 160px (frame width)</small>
                        </div>
                        
                        <div class="form-group">
                            <label class="form-label">ROI Height (px)</label>
                            <input type="number" class="form-control" id="height" 
                                   value=")rawliteral";
    html += String(settings.roi_height);
    html += R"rawliteral(" 
                                   min="10" max="120">
                            <small>Maximum: 120px (frame height)</small>
                        </div>
                        
                        <div class="form-group">
                            <label class="form-label">X Offset (px)</label>
                            <input type="number" class="form-control" id="x" 
                                   value=")rawliteral";
    html += String(settings.roi_x);
    html += R"rawliteral(" 
                                   min="0" max="160">
                            <small>Horizontal position from left</small>
                        </div>
                        
                        <div class="form-group">
                            <label class="form-label">Y Offset (px)</label>
                            <input type="number" class="form-control" id="y" 
                                   value=")rawliteral";
    html += String(settings.roi_y);
    html += R"rawliteral(" 
                                   min="0" max="120">
                            <small>Vertical position from top</small>
                        </div>
                        
                        <div class="form-group">
                            <div style="background: #d1ecf1; border-left: 4px solid #0c5460; padding: 15px;">
                                <strong>💡 ROI Tips:</strong>
                                <ul style="margin-top: 5px; color: #0c5460; padding-left: 20px;">
                                    <li>Position ROI on the parking spot area</li>
                                    <li>Smaller ROI = faster processing</li>
                                    <li>Ensure ROI fits within 160×120 frame</li>
                                </ul>
                            </div>
                        </div>
                        
                        <button type="submit" class="btn btn-wide">
                            🎯 Update ROI Settings
                        </button>
                    </form>
                </div>
            </div>
        </div>
        
        <div class="card mt-4">
            <div class="card-header">
                <h3>Visual Representation</h3>
            </div>
            <div class="card-body">
                <div style="position: relative; width: 320px; height: 240px; background: #f8f9fa; border: 2px solid #dee2e6; border-radius: var(--border-radius); margin: 0 auto;">
                    <div style="position: absolute; left: )rawliteral";
    html += String(settings.roi_x * 2);
    html += R"rawliteral(px; top: )rawliteral";
    html += String(settings.roi_y * 2);
    html += R"rawliteral(px; width: )rawliteral";
    html += String(settings.roi_width * 2);
    html += R"rawliteral(px; height: )rawliteral";
    html += String(settings.roi_height * 2);
    html += R"rawliteral(px; background: rgba(67, 97, 238, 0.3); border: 2px solid #4361ee; border-radius: 4px;">
                        <div style="position: absolute; top: 50%; left: 50%; transform: translate(-50%, -50%); color: #4361ee; font-weight: bold;">
                            ROI
                        </div>
                    </div>
                    <div style="position: absolute; bottom: 10px; right: 10px; color: #666; font-size: 0.8rem;">
                        160×120 (scaled 2x)
                    </div>
                </div>
                <p class="text-center mt-3" style="color: #666;">
                    Visual representation of the ROI area (scaled for visibility)
                </p>
            </div>
        </div>
        
        <script>
            async function saveRoiSettings() {
                const width = parseInt(document.getElementById('width').value);
                const height = parseInt(document.getElementById('height').value);
                const x = parseInt(document.getElementById('x').value);
                const y = parseInt(document.getElementById('y').value);
                
                // Validate ROI fits within frame
                if (x + width > 160) {
                    showNotification('ROI exceeds frame width!', 'error');
                    return;
                }
                
                if (y + height > 120) {
                    showNotification('ROI exceeds frame height!', 'error');
                    return;
                }
                
                const formData = new FormData();
                formData.append('width', width);
                formData.append('height', height);
                formData.append('x', x);
                formData.append('y', y);
                
                await saveSettings('/save_roi', formData);
            }
        </script>
    )rawliteral";
    
    html += getFooter();
    return html;
}