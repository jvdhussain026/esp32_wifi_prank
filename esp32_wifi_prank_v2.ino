#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <EEPROM.h>
#include <ArduinoJson.h>
#include <Update.h>
#include <vector>

// ==================== CONFIGURATION ====================
// Change these values to customize your setup
#define DEFAULT_AP_NAME "Free_Public_WiFi"
#define DEFAULT_ADMIN_PASSWORD "26062008"
#define DEFAULT_PRANK_TYPE 0
#define ENABLE_PRANK true
#define ADMIN_TIMEOUT 300000 // 5 minutes admin session timeout
// =======================================================

// EEPROM addresses for configuration
#define EEPROM_SIZE 4096
#define CONFIG_START 0

// DNS and Web Server
const byte DNS_PORT = 53;
DNSServer dnsServer;
WebServer webServer(80);

// Default configuration
struct Config {
  char apName[24];
  char adminPassword[24];
  char htmlContent[2048]; // Reduced for memory optimization
  int prankType;
  bool enablePrank;
  uint32_t userCount;
  uint32_t startTime;
  uint32_t totalConnections;
  char lastClientMAC[18];
  char lastClientIP[16];
};

Config config;

// Connected clients tracking
struct ClientInfo {
  String mac;
  String ip;
  String hostname;
  uint32_t firstSeen;
  uint32_t lastSeen;
  uint32_t requests;
};

std::vector<ClientInfo> connectedClients;
uint32_t totalUsers = 0;

// Admin session management
String adminSessionToken = "";
unsigned long adminSessionStart = 0;

// System logs
String systemLogs = "";

// Default HTML templates
const char defaultPrankHTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>Free WiFi Portal</title>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
        body {
            font-family: Arial, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            margin: 0;
            padding: 20px;
            color: white;
            text-align: center;
        }
        .container {
            max-width: 600px;
            margin: 50px auto;
            background: rgba(255,255,255,0.1);
            padding: 30px;
            border-radius: 15px;
            backdrop-filter: blur(10px);
        }
        .prank-image {
            max-width: 100%;
            border-radius: 10px;
            margin: 20px 0;
        }
        .message {
            font-size: 24px;
            margin: 20px 0;
            font-weight: bold;
        }
        .submessage {
            font-size: 16px;
            opacity: 0.8;
        }
        .loading {
            margin: 20px 0;
        }
        .ad {
            background: rgba(255,255,255,0.2);
            padding: 15px;
            border-radius: 10px;
            margin: 20px 0;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🎉 FREE WIFI ACCESS 🎉</h1>
        <div class="message" id="mainMessage">Congratulations! You've found free WiFi!</div>
        
        <div class="loading">
            <div>Connecting to network...</div>
            <div style="margin: 10px 0;">
                <progress value="50" max="100" style="width: 80%; height: 20px;"></progress>
            </div>
        </div>
        
        <div class="ad">
            <h3>📱 Special Offer! 📱</h3>
            <p>Get unlimited data for just ₹499/month!</p>
            <p><small>Terms and conditions apply</small></p>
        </div>
        
        <div class="submessage">
            <p>🔒 Your connection is being secured...</p>
            <p>⏳ Please wait while we configure your access</p>
        </div>
        
        <div style="margin-top: 30px; font-size: 12px; opacity: 0.6;">
            Connection established: <span id="connectionTime">0</span> seconds
        </div>
    </div>

    <script>
        let startTime = Date.now();
        function updateTimer() {
            let elapsed = Math.floor((Date.now() - startTime) / 1000);
            document.getElementById('connectionTime').textContent = elapsed;
            
            // Change message after 10 seconds
            if (elapsed === 10) {
                document.getElementById('mainMessage').textContent = "Just kidding! April Fools! 🤣";
            }
            
            // Redirect after 15 seconds
            if (elapsed === 15) {
                window.location.href = "http://www.google.com";
            }
        }
        setInterval(updateTimer, 1000);
        
        // Fake loading animation
        let progress = 50;
        setInterval(() => {
            if (progress < 95) {
                progress += Math.random() * 5;
                document.querySelector('progress').value = progress;
            }
        }, 1000);
    </script>
</body>
</html>
)rawliteral";

const char aprilFoolHTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>April Fools! 🤣</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            background: #ff6b6b;
            margin: 0;
            padding: 20px;
            text-align: center;
            color: white;
        }
        .container {
            max-width: 600px;
            margin: 100px auto;
        }
        h1 {
            font-size: 48px;
            margin-bottom: 30px;
        }
        .emoji {
            font-size: 80px;
            margin: 20px 0;
        }
        .message {
            font-size: 24px;
            margin: 20px 0;
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="emoji">🎉🤣</div>
        <h1>APRIL FOOLS!</h1>
        <div class="message">You've been pranked by Javed's ESP32!</div>
        <div class="message">Free WiFi? Think again! 😜</div>
        <div style="margin-top: 50px; font-size: 16px;">
            <p>This is a harmless prank WiFi portal</p>
            <p>No data has been collected or stored</p>
        </div>
    </div>
</body>
</html>
)rawliteral";

const char fakeVirusHTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>Security Alert</title>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
        body {
            font-family: 'Courier New', monospace;
            background: #000;
            color: #0f0;
            margin: 0;
            padding: 20px;
            overflow: hidden;
        }
        .terminal {
            background: #000;
            padding: 20px;
            border: 1px solid #0f0;
        }
        .blink {
            animation: blink 1s infinite;
        }
        @keyframes blink {
            50% { opacity: 0; }
        }
        .scan-line {
            position: fixed;
            top: 0;
            left: 0;
            width: 100%;
            height: 2px;
            background: #0f0;
            animation: scan 3s linear infinite;
        }
        @keyframes scan {
            0% { top: 0; }
            100% { top: 100%; }
        }
    </style>
</head>
<body>
    <div class="scan-line"></div>
    <div class="terminal">
        <h1>⚠️ SECURITY ALERT ⚠️</h1>
        <p>> Scanning network...</p>
        <p>> VIRUS DETECTED: Trojan.Win32.Prank</p>
        <p>> System compromise: 85%</p>
        <p>> Isolating threat...</p>
        <p class="blink">> EMERGENCY SHUTDOWN IN: <span id="countdown">10</span></p>
        <p>> Just kidding! This is a prank! 🤣</p>
    </div>

    <script>
        let count = 10;
        setInterval(() => {
            count--;
            document.getElementById('countdown').textContent = count;
            if (count <= 0) {
                document.body.innerHTML = '<div style="text-align: center; padding: 100px; background: #000; color: #0f0;"><h1>🎉 APRIL FOOLS! 🎉</h1><p>Your device is safe! 😊</p></div>';
            }
        }, 1000);
    </script>
</body>
</html>
)rawliteral";

const char fakeUpdateHTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>System Update</title>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
        body {
            font-family: Arial, sans-serif;
            background: #0078d7;
            margin: 0;
            padding: 0;
            color: white;
        }
        .update-screen {
            max-width: 800px;
            margin: 100px auto;
            text-align: center;
        }
        .progress-container {
            background: rgba(255,255,255,0.2);
            border-radius: 10px;
            padding: 20px;
            margin: 30px 0;
        }
        .progress-bar {
            background: rgba(255,255,255,0.3);
            border-radius: 5px;
            overflow: hidden;
        }
        .progress {
            background: #00bc0e;
            height: 30px;
            width: 0%;
            transition: width 0.3s;
        }
        .update-info {
            margin: 20px 0;
        }
    </style>
</head>
<body>
    <div class="update-screen">
        <h1>🔄 Windows Update</h1>
        <div class="update-info">
            <p>Installing update 13 of 156...</p>
            <p>Do not turn off your device</p>
        </div>
        
        <div class="progress-container">
            <div class="progress-bar">
                <div class="progress" id="updateProgress"></div>
            </div>
            <p id="progressText">0% Complete</p>
        </div>
        
        <div class="update-info">
            <p>This may take several minutes. Your device will restart several times.</p>
        </div>
    </div>

    <script>
        let progress = 0;
        function updateProgress() {
            progress += Math.random() * 5;
            if (progress > 100) progress = 100;
            
            document.getElementById('updateProgress').style.width = progress + '%';
            document.getElementById('progressText').textContent = Math.round(progress) + '% Complete';
            
            if (progress < 100) {
                setTimeout(updateProgress, 500);
            } else {
                setTimeout(() => {
                    document.body.innerHTML = '<div style="text-align: center; padding: 100px;"><h1>🤣 GOTCHA!</h1><p>This was a prank update screen!</p></div>';
                }, 1000);
            }
        }
        updateProgress();
    </script>
</body>
</html>
)rawliteral";

// Login page HTML
const char loginHTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>Admin Login - ESP32 Prank Portal</title>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            margin: 0;
            padding: 0;
            display: flex;
            justify-content: center;
            align-items: center;
            min-height: 100vh;
        }
        .login-container {
            background: white;
            padding: 40px;
            border-radius: 15px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.3);
            width: 100%;
            max-width: 400px;
        }
        .logo {
            text-align: center;
            margin-bottom: 30px;
        }
        .logo h1 {
            color: #333;
            margin: 0;
            font-size: 24px;
        }
        .form-group {
            margin-bottom: 20px;
        }
        label {
            display: block;
            margin-bottom: 5px;
            color: #555;
            font-weight: 600;
        }
        input {
            width: 100%;
            padding: 12px;
            border: 2px solid #ddd;
            border-radius: 8px;
            font-size: 16px;
            transition: border-color 0.3s;
        }
        input:focus {
            outline: none;
            border-color: #667eea;
        }
        .btn {
            width: 100%;
            padding: 12px;
            background: #667eea;
            color: white;
            border: none;
            border-radius: 8px;
            font-size: 16px;
            cursor: pointer;
            transition: background 0.3s;
        }
        .btn:hover {
            background: #5a6fd8;
        }
        .error {
            color: #e53e3e;
            text-align: center;
            margin-top: 10px;
            display: none;
        }
    </style>
</head>
<body>
    <div class="login-container">
        <div class="logo">
            <h1>🔧 ESP32 Admin</h1>
            <p>Prank Portal Control Panel</p>
        </div>
        <form id="loginForm">
            <div class="form-group">
                <label for="password">Admin Password</label>
                <input type="password" id="password" name="password" placeholder="Enter admin password" required>
            </div>
            <button type="submit" class="btn">Login</button>
            <div id="errorMessage" class="error">Invalid password!</div>
        </form>
    </div>

    <script>
        document.getElementById('loginForm').addEventListener('submit', function(e) {
            e.preventDefault();
            const password = document.getElementById('password').value;
            
            fetch('/api/login', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify({password: password})
            })
            .then(response => response.json())
            .then(data => {
                if (data.success) {
                    window.location.href = '/admin';
                } else {
                    document.getElementById('errorMessage').style.display = 'block';
                }
            });
        });
    </script>
</body>
</html>
)rawliteral";

// Enhanced Admin panel HTML
const char adminHTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>ESP32 Prank Portal - Admin</title>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
        :root {
            --primary: #667eea;
            --secondary: #764ba2;
            --dark: #2d3748;
            --light: #f7fafc;
            --danger: #e53e3e;
            --success: #38a169;
            --warning: #dd6b20;
        }
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, var(--primary) 0%, var(--secondary) 100%);
            min-height: 100vh;
            color: var(--dark);
        }
        .container {
            max-width: 1200px;
            margin: 0 auto;
            padding: 20px;
        }
        .header {
            background: white;
            padding: 20px;
            border-radius: 10px;
            box-shadow: 0 4px 6px rgba(0,0,0,0.1);
            margin-bottom: 20px;
            display: flex;
            justify-content: space-between;
            align-items: center;
        }
        .header-content {
            flex: 1;
        }
        .logout-btn {
            background: var(--danger);
            color: white;
            border: none;
            padding: 10px 20px;
            border-radius: 5px;
            cursor: pointer;
        }
        .tabs {
            display: flex;
            background: white;
            border-radius: 10px;
            overflow: hidden;
            margin-bottom: 20px;
            flex-wrap: wrap;
        }
        .tab {
            padding: 15px 20px;
            cursor: pointer;
            border: none;
            background: transparent;
            flex: 1;
            min-width: 120px;
            font-size: 14px;
            transition: all 0.3s;
        }
        .tab.active {
            background: var(--primary);
            color: white;
        }
        .tab-content {
            display: none;
            background: white;
            padding: 30px;
            border-radius: 10px;
            box-shadow: 0 4px 6px rgba(0,0,0,0.1);
        }
        .tab-content.active {
            display: block;
        }
        .card {
            background: var(--light);
            padding: 20px;
            border-radius: 8px;
            margin-bottom: 20px;
            border-left: 4px solid var(--primary);
        }
        .stats-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 20px;
            margin-bottom: 30px;
        }
        .stat-card {
            background: white;
            padding: 20px;
            border-radius: 8px;
            text-align: center;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
        }
        .stat-number {
            font-size: 2em;
            font-weight: bold;
            color: var(--primary);
        }
        .stat-label {
            font-size: 0.9em;
            color: var(--dark);
            opacity: 0.7;
        }
        .form-group {
            margin-bottom: 20px;
        }
        label {
            display: block;
            margin-bottom: 5px;
            font-weight: 600;
        }
        input, textarea, select {
            width: 100%;
            padding: 10px;
            border: 1px solid #ddd;
            border-radius: 5px;
            font-size: 16px;
        }
        textarea {
            height: 200px;
            font-family: monospace;
        }
        .btn {
            padding: 12px 24px;
            border: none;
            border-radius: 5px;
            cursor: pointer;
            font-size: 16px;
            transition: all 0.3s;
            margin-right: 10px;
            margin-bottom: 10px;
        }
        .btn-primary {
            background: var(--primary);
            color: white;
        }
        .btn-danger {
            background: var(--danger);
            color: white;
        }
        .btn-success {
            background: var(--success);
            color: white;
        }
        .btn-warning {
            background: var(--warning);
            color: white;
        }
        .client-list {
            max-height: 400px;
            overflow-y: auto;
        }
        .client-item {
            background: var(--light);
            padding: 15px;
            margin-bottom: 10px;
            border-radius: 5px;
            border-left: 4px solid var(--success);
        }
        .upload-area {
            border: 2px dashed #ddd;
            padding: 40px;
            text-align: center;
            border-radius: 8px;
            margin-bottom: 20px;
        }
        .template-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
            gap: 15px;
            margin-bottom: 20px;
        }
        .template-card {
            background: var(--light);
            padding: 15px;
            border-radius: 8px;
            cursor: pointer;
            transition: all 0.3s;
            border: 2px solid transparent;
        }
        .template-card:hover {
            border-color: var(--primary);
        }
        .template-card.active {
            border-color: var(--primary);
            background: #e6f0ff;
        }
        .logs {
            background: #1a202c;
            color: #cbd5e0;
            padding: 15px;
            border-radius: 5px;
            font-family: monospace;
            height: 300px;
            overflow-y: auto;
            white-space: pre-wrap;
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <div class="header-content">
                <h1>🎯 ESP32 Prank Portal - Admin Panel</h1>
                <p>Welcome to Javed's Advanced Prank System v2.0</p>
            </div>
            <button class="logout-btn" onclick="logout()">Logout</button>
        </div>

        <div class="tabs">
            <button class="tab active" onclick="switchTab('overview')">📊 Overview</button>
            <button class="tab" onclick="switchTab('configuration')">⚙️ Configuration</button>
            <button class="tab" onclick="switchTab('templates')">🎨 Templates</button>
            <button class="tab" onclick="switchTab('clients')">👥 Clients</button>
            <button class="tab" onclick="switchTab('editor')">📝 Editor</button>
            <button class="tab" onclick="switchTab('upload')">📤 Upload</button>
            <button class="tab" onclick="switchTab('logs')">📋 Logs</button>
        </div>

        <div id="overview" class="tab-content active">
            <div class="stats-grid">
                <div class="stat-card">
                    <div class="stat-number" id="uptime">0</div>
                    <div class="stat-label">Uptime</div>
                </div>
                <div class="stat-card">
                    <div class="stat-number" id="totalUsers">0</div>
                    <div class="stat-label">Total Users</div>
                </div>
                <div class="stat-card">
                    <div class="stat-number" id="activeClients">0</div>
                    <div class="stat-label">Active Clients</div>
                </div>
                <div class="stat-card">
                    <div class="stat-number" id="memory">0%</div>
                    <div class="stat-label">Memory Usage</div>
                </div>
                <div class="stat-card">
                    <div class="stat-number" id="totalConnections">0</div>
                    <div class="stat-label">Total Connections</div>
                </div>
                <div class="stat-card">
                    <div class="stat-number" id="prankType">0</div>
                    <div class="stat-label">Active Prank</div>
                </div>
            </div>
            
            <div class="card">
                <h3>System Information</h3>
                <p><strong>AP Name:</strong> <span id="apName">Loading...</span></p>
                <p><strong>IP Address:</strong> <span id="ipAddress">Loading...</span></p>
                <p><strong>ESP32 ID:</strong> <span id="espId">Loading...</span></p>
                <p><strong>Last Client:</strong> <span id="lastClient">Loading...</span></p>
                <p><strong>Firmware Version:</strong> Javed's Prank Portal v2.0</p>
            </div>
        </div>

        <div id="configuration" class="tab-content">
            <form id="configForm">
                <div class="form-group">
                    <label for="apName">Access Point Name</label>
                    <input type="text" id="apNameInput" name="apName" placeholder="Enter AP Name">
                </div>
                
                <div class="form-group">
                    <label for="adminPassword">Admin Password</label>
                    <input type="password" id="adminPassword" name="adminPassword" placeholder="Enter Admin Password">
                </div>
                
                <div class="form-group">
                    <label for="prankType">Prank Type</label>
                    <select id="prankType" name="prankType">
                        <option value="0">Default Free WiFi Prank</option>
                        <option value="1">April Fools Special</option>
                        <option value="2">Fake Virus Alert</option>
                        <option value="3">Fake System Update</option>
                        <option value="4">Custom HTML</option>
                    </select>
                </div>
                
                <div class="form-group">
                    <label>
                        <input type="checkbox" id="enablePrank" name="enablePrank"> Enable Prank Portal
                    </label>
                </div>
                
                <button type="submit" class="btn btn-primary">Save Configuration</button>
                <button type="button" class="btn btn-danger" onclick="resetConfig()">Reset to Defaults</button>
                <button type="button" class="btn btn-warning" onclick="restartDevice()">Restart ESP32</button>
            </form>
        </div>

        <div id="templates" class="tab-content">
            <h3>Select Prank Template</h3>
            <div class="template-grid">
                <div class="template-card" onclick="selectTemplate(0)">
                    <h4>Default Free WiFi</h4>
                    <p>Classic free WiFi prank with fake loading</p>
                </div>
                <div class="template-card" onclick="selectTemplate(1)">
                    <h4>April Fools</h4>
                    <p>Simple and direct April Fools message</p>
                </div>
                <div class="template-card" onclick="selectTemplate(2)">
                    <h4>Fake Virus Alert</h4>
                    <p>Scary terminal-style virus warning</p>
                </div>
                <div class="template-card" onclick="selectTemplate(3)">
                    <h4>Fake System Update</h4>
                    <p>Windows-style update screen</p>
                </div>
            </div>
            <button class="btn btn-primary" onclick="applyTemplate()">Apply Selected Template</button>
        </div>

        <div id="clients" class="tab-content">
            <h3>Connected Clients</h3>
            <div id="clientList" class="client-list">
                <!-- Client list will be populated here -->
            </div>
            <button class="btn btn-primary" onclick="refreshClients()">Refresh List</button>
            <button class="btn btn-danger" onclick="clearClients()">Clear All Clients</button>
        </div>

        <div id="editor" class="tab-content">
            <div class="form-group">
                <label for="htmlEditor">Custom HTML Editor</label>
                <textarea id="htmlEditor" placeholder="Enter your custom HTML here..."></textarea>
            </div>
            <div class="form-group">
                <label>Image URL (optional):</label>
                <input type="text" id="imageUrl" placeholder="https://example.com/image.jpg">
                <small>Note: You can use online image URLs or base64 encoded images</small>
            </div>
            <button class="btn btn-primary" onclick="saveHTML()">Save HTML</button>
            <button class="btn" onclick="previewHTML()">Preview</button>
            <button class="btn btn-success" onclick="insertImage()">Insert Image Tag</button>
        </div>

        <div id="upload" class="tab-content">
            <div class="upload-area">
                <h3>Upload HTML File</h3>
                <p>Drag & drop your HTML file here or click to browse</p>
                <input type="file" id="fileInput" accept=".html,.htm,.txt" style="display: none;">
                <button class="btn btn-primary" onclick="document.getElementById('fileInput').click()">Select File</button>
            </div>
            <div id="uploadStatus"></div>
        </div>

        <div id="logs" class="tab-content">
            <h3>System Logs</h3>
            <div class="logs" id="systemLogs">
                Loading logs...
            </div>
            <button class="btn btn-primary" onclick="refreshLogs()">Refresh Logs</button>
            <button class="btn btn-danger" onclick="clearLogs()">Clear Logs</button>
        </div>
    </div>

    <script>
        let selectedTemplate = 0;

        function switchTab(tabName) {
            document.querySelectorAll('.tab-content').forEach(tab => {
                tab.classList.remove('active');
            });
            document.querySelectorAll('.tab').forEach(tab => {
                tab.classList.remove('active');
            });
            
            document.getElementById(tabName).classList.add('active');
            event.target.classList.add('active');
            
            if (tabName === 'logs') {
                refreshLogs();
            }
        }

        function selectTemplate(templateId) {
            selectedTemplate = templateId;
            document.querySelectorAll('.template-card').forEach(card => {
                card.classList.remove('active');
            });
            event.target.classList.add('active');
        }

        function applyTemplate() {
            fetch('/api/apply-template', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify({template: selectedTemplate})
            }).then(response => {
                if (response.ok) {
                    alert('Template applied successfully!');
                    updateStats();
                }
            });
        }

        function updateStats() {
            fetch('/api/stats')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('uptime').textContent = data.uptime;
                    document.getElementById('totalUsers').textContent = data.totalUsers;
                    document.getElementById('activeClients').textContent = data.activeClients;
                    document.getElementById('memory').textContent = data.memoryUsage + '%';
                    document.getElementById('apName').textContent = data.apName;
                    document.getElementById('ipAddress').textContent = data.ipAddress;
                    document.getElementById('espId').textContent = data.espId;
                    document.getElementById('totalConnections').textContent = data.totalConnections;
                    document.getElementById('lastClient').textContent = data.lastClient;
                    document.getElementById('prankType').textContent = data.prankType;
                });
            
            fetch('/api/clients')
                .then(response => response.json())
                .then(data => {
                    const clientList = document.getElementById('clientList');
                    clientList.innerHTML = '';
                    data.clients.forEach(client => {
                        const clientDiv = document.createElement('div');
                        clientDiv.className = 'client-item';
                        clientDiv.innerHTML = `
                            <strong>${client.hostname || 'Unknown Device'}</strong><br>
                            MAC: ${client.mac} | IP: ${client.ip}<br>
                            Connected: ${client.connectedTime} | Requests: ${client.requests}
                        `;
                        clientList.appendChild(clientDiv);
                    });
                });
        }

        document.getElementById('configForm').addEventListener('submit', function(e) {
            e.preventDefault();
            const formData = new FormData(this);
            fetch('/api/config', {
                method: 'POST',
                body: formData
            }).then(response => {
                if (response.ok) {
                    alert('Configuration saved successfully!');
                    updateStats();
                }
            });
        });

        function saveHTML() {
            const htmlContent = document.getElementById('htmlEditor').value;
            fetch('/api/save-html', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify({html: htmlContent})
            }).then(response => {
                if (response.ok) {
                    alert('HTML saved successfully!');
                }
            });
        }

        function previewHTML() {
            const htmlContent = document.getElementById('htmlEditor').value;
            const previewWindow = window.open('', '_blank');
            previewWindow.document.write(htmlContent);
        }

        function insertImage() {
            const imageUrl = document.getElementById('imageUrl').value;
            if (imageUrl) {
                const imageTag = `<img src="${imageUrl}" alt="Prank Image" style="max-width: 100%; border-radius: 10px; margin: 10px 0;">`;
                const editor = document.getElementById('htmlEditor');
                editor.value += '\n' + imageTag;
            }
        }

        function resetConfig() {
            if (confirm('Are you sure you want to reset all configuration to defaults?')) {
                fetch('/api/reset', {method: 'POST'})
                    .then(response => {
                        if (response.ok) {
                            alert('Configuration reset successfully!');
                            location.reload();
                        }
                    });
            }
        }

        function restartDevice() {
            if (confirm('Are you sure you want to restart the ESP32?')) {
                fetch('/api/restart', {method: 'POST'})
                    .then(response => {
                        if (response.ok) {
                            alert('ESP32 is restarting...');
                        }
                    });
            }
        }

        function refreshClients() {
            updateStats();
        }

        function clearClients() {
            if (confirm('Clear all client records?')) {
                fetch('/api/clear-clients', {method: 'POST'})
                    .then(response => {
                        if (response.ok) {
                            alert('Clients cleared!');
                            updateStats();
                        }
                    });
            }
        }

        function refreshLogs() {
            fetch('/api/logs')
                .then(response => response.text())
                .then(data => {
                    document.getElementById('systemLogs').textContent = data;
                });
        }

        function clearLogs() {
            if (confirm('Clear all logs?')) {
                fetch('/api/clear-logs', {method: 'POST'})
                    .then(response => {
                        if (response.ok) {
                            refreshLogs();
                        }
                    });
            }
        }

        function logout() {
            fetch('/api/logout', {method: 'POST'})
                .then(() => {
                    window.location.href = '/login';
                });
        }

        // File upload handling
        document.getElementById('fileInput').addEventListener('change', function(e) {
            const file = e.target.files[0];
            if (file) {
                const reader = new FileReader();
                reader.onload = function(e) {
                    document.getElementById('htmlEditor').value = e.target.result;
                    document.getElementById('uploadStatus').innerHTML = '<div style="color: green;">File loaded successfully! Click "Save HTML" to apply.</div>';
                };
                reader.readAsText(file);
            }
        });

        // Update stats every 5 seconds
        setInterval(updateStats, 5000);
        updateStats();

        // Load current configuration
        fetch('/api/config')
            .then(response => response.json())
            .then(data => {
                document.getElementById('apNameInput').value = data.apName || '';
                document.getElementById('prankType').value = data.prankType || '0';
                document.getElementById('enablePrank').checked = data.enablePrank || false;
                document.getElementById('htmlEditor').value = data.htmlContent || '';
            });

        // Check session every minute
        setInterval(() => {
            fetch('/api/check-session')
                .then(response => {
                    if (!response.ok) {
                        window.location.href = '/login';
                    }
                });
        }, 60000);
    </script>
</body>
</html>
)rawliteral";

// ==================== FIXED FUNCTIONS ====================

// Get current timestamp for logging
String getTimestamp() {
  unsigned long uptime = millis() / 1000;
  uint32_t hours = uptime / 3600;
  uint32_t minutes = (uptime % 3600) / 60;
  uint32_t seconds = uptime % 60;
  char timestamp[20];
  snprintf(timestamp, sizeof(timestamp), "[%02lu:%02lu:%02lu]", hours, minutes, seconds);
  return String(timestamp);
}

// Fixed addLog function
void addLog(String message) {
  String logEntry = getTimestamp() + " " + message;
  Serial.println(logEntry);
  
  // Keep only last 20 lines in memory
  systemLogs += logEntry + "\n";
  int newlineCount = 0;
  for (int i = systemLogs.length() - 1; i >= 0; i--) {
    if (systemLogs.charAt(i) == '\n') {
      newlineCount++;
      if (newlineCount > 20) {
        systemLogs = systemLogs.substring(i + 1);
        break;
      }
    }
  }
}

// Configuration management
void saveConfig() {
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.put(CONFIG_START, config);
  EEPROM.commit();
  EEPROM.end();
  addLog("Configuration saved to EEPROM");
}

void loadConfig() {
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.get(CONFIG_START, config);
  EEPROM.end();

  // Initialize with defaults if config is invalid
  if (strlen(config.apName) == 0) {
    strcpy(config.apName, DEFAULT_AP_NAME);
    strcpy(config.adminPassword, DEFAULT_ADMIN_PASSWORD);
    strcpy(config.htmlContent, defaultPrankHTML);
    config.prankType = DEFAULT_PRANK_TYPE;
    config.enablePrank = ENABLE_PRANK;
    config.userCount = 0;
    config.startTime = millis();
    config.totalConnections = 0;
    strcpy(config.lastClientMAC, "00:00:00:00:00:00");
    strcpy(config.lastClientIP, "0.0.0.0");
    saveConfig();
  }
  addLog("Configuration loaded from EEPROM");
}

// Generate session token
String generateSessionToken() {
  String token = "";
  for (int i = 0; i < 32; i++) {
    token += char(random(65, 91)); // A-Z
  }
  return token;
}

// Check if admin is authenticated
bool isAuthenticated() {
  if (adminSessionToken.length() == 0) return false;
  if (millis() - adminSessionStart > ADMIN_TIMEOUT) {
    adminSessionToken = "";
    addLog("Admin session expired");
    return false;
  }
  return true;
}

// Get client hostname
String getClientHostname(String mac) {
  if (mac == "Unknown") return "Unknown Device";
  
  // Create a simple hostname based on MAC
  String shortMac = mac.substring(mac.length() - 5);
  shortMac.replace(":", "");
  return "Device_" + shortMac;
}

// Get client MAC (simplified version for ESP32)
String getClientMAC(String ip) {
  // For ESP32 in AP mode, we can't easily map IP to MAC
  // Return a simplified version based on IP for tracking
  if (ip == "0.0.0.0") return "Unknown";
  
  // Create a pseudo-MAC from IP for tracking
  String pseudoMAC = "CL:";
  pseudoMAC += ip.substring(ip.lastIndexOf('.') + 1);
  pseudoMAC += ":XX:XX";
  return pseudoMAC;
}

// Update client tracking
void updateClientTracking() {
  String clientIP = webServer.client().remoteIP().toString();
  
  // Get MAC address using alternative method
  String clientMAC = getClientMAC(clientIP);
  
  // Update last client info
  strcpy(config.lastClientMAC, clientMAC.c_str());
  strcpy(config.lastClientIP, clientIP.c_str());
  config.totalConnections++;
  
  // Check if client already exists
  bool found = false;
  for (auto& clientInfo : connectedClients) {
    if (clientInfo.ip == clientIP) {
      clientInfo.lastSeen = millis();
      clientInfo.requests++;
      found = true;
      break;
    }
  }
  
  // Add new client if not found
  if (!found && clientMAC != "Unknown") {
    ClientInfo newClient;
    newClient.mac = clientMAC;
    newClient.ip = clientIP;
    newClient.hostname = getClientHostname(clientMAC);
    newClient.firstSeen = millis();
    newClient.lastSeen = millis();
    newClient.requests = 1;
    connectedClients.push_back(newClient);
    config.userCount++;
    addLog("New client connected: " + clientMAC + " (" + newClient.hostname + ")");
  }
  
  saveConfig();
}

// Clean up old clients (older than 1 hour)
void cleanupOldClients() {
  unsigned long currentTime = millis();
  connectedClients.erase(
    std::remove_if(connectedClients.begin(), connectedClients.end(),
      [currentTime](const ClientInfo& client) {
        return (currentTime - client.lastSeen) > 3600000; // 1 hour
      }),
    connectedClients.end()
  );
}

// Memory check function
void checkMemory() {
  static unsigned long lastMemCheck = 0;
  if (millis() - lastMemCheck > 30000) { // Every 30 seconds
    addLog("Free heap: " + String(ESP.getFreeHeap()) + " bytes");
    lastMemCheck = millis();
  }
}

// Input validation
bool isValidAPName(String name) {
  return name.length() > 0 && name.length() <= 23;
}

bool isValidPassword(String password) {
  return password.length() >= 4 && password.length() <= 23;
}

// Get prank HTML based on type
String getPrankHTML() {
  switch(config.prankType) {
    case 0: return String(defaultPrankHTML);
    case 1: return String(aprilFoolHTML);
    case 2: return String(fakeVirusHTML);
    case 3: return String(fakeUpdateHTML);
    case 4: return String(config.htmlContent);
    default: return String(defaultPrankHTML);
  }
}

// Setup WiFi Access Point
void setupWiFi() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(config.apName);
  addLog("Access Point Started: " + String(config.apName));
  addLog("AP IP Address: " + WiFi.softAPIP().toString());
}

// Setup DNS Server
void setupDNS() {
  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
  addLog("DNS Server started on port " + String(DNS_PORT));
}

// Setup Web Server Routes
void setupWebServer() {
  // Login routes
  webServer.on("/login", HTTP_GET, []() {
    webServer.send(200, "text/html", loginHTML);
  });

  webServer.on("/api/login", HTTP_POST, []() {
    String body = webServer.arg("plain");
    DynamicJsonDocument doc(256);
    deserializeJson(doc, body);
    
    String password = doc["password"];
    if (password == config.adminPassword) {
      adminSessionToken = generateSessionToken();
      adminSessionStart = millis();
      
      DynamicJsonDocument response(128);
      response["success"] = true;
      response["token"] = adminSessionToken;
      
      String jsonResponse;
      serializeJson(response, jsonResponse);
      webServer.send(200, "application/json", jsonResponse);
      addLog("Admin login successful");
    } else {
      DynamicJsonDocument response(128);
      response["success"] = false;
      String jsonResponse;
      serializeJson(response, jsonResponse);
      webServer.send(401, "application/json", jsonResponse);
      addLog("Failed admin login attempt");
    }
  });

  // Admin panel
  webServer.on("/admin", HTTP_GET, []() {
    if (!isAuthenticated()) {
      webServer.sendHeader("Location", "/login");
      webServer.send(302, "text/plain", "Redirecting to login");
      return;
    }
    webServer.send(200, "text/html", adminHTML);
  });

  // API routes
  webServer.on("/api/stats", HTTP_GET, []() {
    if (!isAuthenticated()) {
      webServer.send(401, "text/plain", "Unauthorized");
      return;
    }
    
    DynamicJsonDocument doc(1024);
    doc["uptime"] = String((millis() - config.startTime) / 1000) + "s";
    doc["totalUsers"] = config.userCount;
    doc["activeClients"] = connectedClients.size();
    doc["memoryUsage"] = 100 - ((ESP.getFreeHeap() * 100) / ESP.getHeapSize());
    doc["apName"] = config.apName;
    doc["ipAddress"] = WiFi.softAPIP().toString();
    doc["espId"] = String((uint32_t)ESP.getEfuseMac(), HEX);
    doc["totalConnections"] = config.totalConnections;
    doc["lastClient"] = String(config.lastClientMAC) + " (" + String(config.lastClientIP) + ")";
    doc["prankType"] = config.prankType;
    
    String jsonResponse;
    serializeJson(doc, jsonResponse);
    webServer.send(200, "application/json", jsonResponse);
  });

  webServer.on("/api/clients", HTTP_GET, []() {
    if (!isAuthenticated()) {
      webServer.send(401, "text/plain", "Unauthorized");
      return;
    }
    
    DynamicJsonDocument doc(2048);
    JsonArray clients = doc.createNestedArray("clients");
    
    for (const auto& client : connectedClients) {
      JsonObject clientObj = clients.createNestedObject();
      clientObj["mac"] = client.mac;
      clientObj["ip"] = client.ip;
      clientObj["hostname"] = client.hostname;
      clientObj["connectedTime"] = String((millis() - client.firstSeen) / 1000) + "s";
      clientObj["requests"] = client.requests;
    }
    
    String jsonResponse;
    serializeJson(doc, jsonResponse);
    webServer.send(200, "application/json", jsonResponse);
  });

  webServer.on("/api/config", HTTP_GET, []() {
    if (!isAuthenticated()) {
      webServer.send(401, "text/plain", "Unauthorized");
      return;
    }
    
    DynamicJsonDocument doc(4096);
    doc["apName"] = config.apName;
    doc["prankType"] = config.prankType;
    doc["enablePrank"] = config.enablePrank;
    doc["htmlContent"] = config.htmlContent;
    
    String jsonResponse;
    serializeJson(doc, jsonResponse);
    webServer.send(200, "application/json", jsonResponse);
  });

  webServer.on("/api/config", HTTP_POST, []() {
    if (!isAuthenticated()) {
      webServer.send(401, "text/plain", "Unauthorized");
      return;
    }
    
    if (webServer.hasArg("apName")) {
      String newApName = webServer.arg("apName");
      if (isValidAPName(newApName)) {
        strcpy(config.apName, newApName.c_str());
      }
    }
    if (webServer.hasArg("adminPassword") && webServer.arg("adminPassword").length() > 0) {
      String newPassword = webServer.arg("adminPassword");
      if (isValidPassword(newPassword)) {
        strcpy(config.adminPassword, newPassword.c_str());
      }
    }
    if (webServer.hasArg("prankType")) {
      config.prankType = webServer.arg("prankType").toInt();
    }
    config.enablePrank = webServer.hasArg("enablePrank");
    
    saveConfig();
    
    // Restart AP with new name if changed
    WiFi.softAP(config.apName);
    
    webServer.send(200, "text/plain", "Configuration saved");
    addLog("Configuration updated via admin panel");
  });

  webServer.on("/api/apply-template", HTTP_POST, []() {
    if (!isAuthenticated()) {
      webServer.send(401, "text/plain", "Unauthorized");
      return;
    }
    
    String body = webServer.arg("plain");
    DynamicJsonDocument doc(256);
    deserializeJson(doc, body);
    
    int templateId = doc["template"];
    config.prankType = templateId;
    saveConfig();
    
    webServer.send(200, "text/plain", "Template applied");
    addLog("Template changed to ID: " + String(templateId));
  });

  webServer.on("/api/save-html", HTTP_POST, []() {
    if (!isAuthenticated()) {
      webServer.send(401, "text/plain", "Unauthorized");
      return;
    }
    
    String body = webServer.arg("plain");
    DynamicJsonDocument doc(4096);
    deserializeJson(doc, body);
    
    String html = doc["html"];
    if (html.length() < sizeof(config.htmlContent)) {
      strcpy(config.htmlContent, html.c_str());
      config.prankType = 4; // Custom HTML mode
      saveConfig();
      webServer.send(200, "text/plain", "HTML saved");
      addLog("Custom HTML content saved");
    } else {
      webServer.send(413, "text/plain", "HTML too large");
    }
  });

  webServer.on("/api/reset", HTTP_POST, []() {
    if (!isAuthenticated()) {
      webServer.send(401, "text/plain", "Unauthorized");
      return;
    }
    
    // Reset to defaults
    strcpy(config.apName, DEFAULT_AP_NAME);
    strcpy(config.adminPassword, DEFAULT_ADMIN_PASSWORD);
    strcpy(config.htmlContent, defaultPrankHTML);
    config.prankType = DEFAULT_PRANK_TYPE;
    config.enablePrank = ENABLE_PRANK;
    saveConfig();
    
    webServer.send(200, "text/plain", "Configuration reset");
    addLog("Configuration reset to defaults");
  });

  webServer.on("/api/restart", HTTP_POST, []() {
    if (!isAuthenticated()) {
      webServer.send(401, "text/plain", "Unauthorized");
      return;
    }
    
    webServer.send(200, "text/plain", "Restarting...");
    addLog("Manual restart initiated via admin panel");
    delay(1000);
    ESP.restart();
  });

  webServer.on("/api/clear-clients", HTTP_POST, []() {
    if (!isAuthenticated()) {
      webServer.send(401, "text/plain", "Unauthorized");
      return;
    }
    
    connectedClients.clear();
    config.userCount = 0;
    saveConfig();
    webServer.send(200, "text/plain", "Clients cleared");
    addLog("Client list cleared via admin panel");
  });

  webServer.on("/api/logs", HTTP_GET, []() {
    if (!isAuthenticated()) {
      webServer.send(401, "text/plain", "Unauthorized");
      return;
    }
    
    webServer.send(200, "text/plain", systemLogs);
  });

  webServer.on("/api/clear-logs", HTTP_POST, []() {
    if (!isAuthenticated()) {
      webServer.send(401, "text/plain", "Unauthorized");
      return;
    }
    
    systemLogs = "";
    addLog("Logs cleared via admin panel");
    webServer.send(200, "text/plain", "Logs cleared");
  });

  webServer.on("/api/check-session", HTTP_GET, []() {
    if (isAuthenticated()) {
      webServer.send(200, "text/plain", "OK");
    } else {
      webServer.send(401, "text/plain", "Unauthorized");
    }
  });

  webServer.on("/api/logout", HTTP_POST, []() {
    adminSessionToken = "";
    webServer.send(200, "text/plain", "Logged out");
    addLog("Admin logged out");
  });

  // Prank portal - main route
  webServer.on("/", HTTP_GET, []() {
    if (!config.enablePrank) {
      webServer.send(200, "text/html", "<html><body><h1>Portal Disabled</h1><p>The prank portal is currently disabled.</p></body></html>");
      return;
    }
    updateClientTracking();
    webServer.send(200, "text/html", getPrankHTML());
  });

  // Captive portal - catch all other requests
  webServer.onNotFound([]() {
    if (config.enablePrank) {
      updateClientTracking();
      webServer.send(200, "text/html", getPrankHTML());
    } else {
      webServer.send(200, "text/html", "<html><body><h1>Portal Disabled</h1><p>The prank portal is currently disabled.</p></body></html>");
    }
  });

  webServer.begin();
  addLog("Web server started on port 80");
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  addLog("=== Javed's ESP32 Prank Portal v2.0 Starting ===");
  
  // Initialize EEPROM and load configuration
  EEPROM.begin(EEPROM_SIZE);
  loadConfig();
  
  // Setup WiFi Access Point
  setupWiFi();
  
  // Setup DNS Server
  setupDNS();
  
  // Setup Web Server
  setupWebServer();
  
  addLog("=== Prank Portal Ready ===");
  addLog("Admin Panel: http://" + WiFi.softAPIP().toString() + "/admin");
  addLog("Default Password: " + String(config.adminPassword));
}

void loop() {
  dnsServer.processNextRequest();
  webServer.handleClient();
  
  // Clean up old clients every 30 seconds
  static unsigned long lastCleanup = 0;
  if (millis() - lastCleanup > 30000) {
    cleanupOldClients();
    checkMemory();
    lastCleanup = millis();
  }
}   