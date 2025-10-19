
# 🎯 ESP32 WiFi Prank Portal v2.0

<div align="center">

![ESP32](https://img.shields.io/badge/ESP32-Development-blue?style=for-the-badge&logo=arduino)
![WiFi](https://img.shields.io/badge/WiFi-Prank_Portal-orange?style=for-the-badge)
![Version](https://img.shields.io/badge/Version-2.0-green?style=for-the-badge)
![Status](https://img.shields.io/badge/Status-Under_Development-yellow?style=for-the-badge)

**Advanced WiFi Prank System with Web Admin Panel**

*Create hilarious captive portal pranks with your ESP32!*

</div>

## ⚠️ Project Status

> **Development Notice**: This project is currently under active development. Features are being tested and improved. Screenshots and additional documentation will be added as development progresses.

## 🚀 Features

### 🎨 Multiple Prank Templates
- **Default Free WiFi**: Fake loading screen with countdown
- **April Fools Special**: Direct and funny prank message
- **Fake Virus Alert**: Terminal-style security warning
- **Fake System Update**: Windows-style update screen
- **Custom HTML**: Upload your own prank pages

### 🔧 Advanced Admin Panel
- **Real-time Statistics**: Uptime, connected clients, memory usage
- **Live Client Monitoring**: Track connected devices with MAC/IP
- **Web-based Configuration**: Change settings without reprogramming
- **HTML Editor**: Built-in code editor with preview
- **File Upload**: Upload custom HTML files directly
- **System Logs**: Detailed activity logging

### 📊 Client Tracking
- Device connection monitoring
- Request counting
- Session duration tracking
- Historical data storage
- Real-time client list

### 🔒 Security Features
- Password-protected admin panel
- Session timeout (5 minutes)
- Input validation
- Secure configuration storage

## 🛠️ Hardware Requirements

- ESP32 Development Board
- Micro-USB Cable
- Computer with Arduino IDE
- Power Source (USB or battery)

## 📋 Software Requirements

- Arduino IDE 2.0+
- ESP32 Board Support
- Required Libraries:
  - WiFi
  - WebServer
  - DNSServer
  - EEPROM
  - ArduinoJson
  - Update

## 🔧 Installation

### 1. Library Installation
```cpp
// Install via Arduino Library Manager:
// - ArduinoJson by Benoit Blanchon
// - WebServer (included with ESP32)
```

### 2. Code Setup
1. Open `esp32_wifi_prank_v2.ino` in Arduino IDE
2. Select ESP32 board from Tools > Board
3. Choose correct COM port
4. Upload the sketch

### 3. First Boot
1. ESP32 will create "Free_Public_WiFi" access point
2. Connect to the WiFi network
3. Open browser to `http://192.168.4.1/admin`
4. Login with default password: `26062008`

## ⚙️ Configuration

### Default Settings
```cpp
#define DEFAULT_AP_NAME "Free_Public_WiFi"
#define DEFAULT_ADMIN_PASSWORD "26062008"
#define DEFAULT_PRANK_TYPE 0
#define ENABLE_PRANK true
```

### EEPROM Storage
- Configuration persists after reboot
- Automatic backup/restore
- Reset to defaults available

## 🎭 Prank Types

### 1. Default Free WiFi
- Fake connection progress
- Countdown timer
- Automatic redirect
- Fake advertisements

### 2. April Fools
- Direct prank message
- Funny emojis
- Harmless notification

### 3. Fake Virus Alert
- Terminal-style interface
- Fake virus scan
- Countdown to "reveal"
- Green-on-black design

### 4. Fake System Update
- Windows-style UI
- Progress bar animation
- Fake update messages
- Comedic reveal

### 5. Custom HTML
- Upload your own designs
- Image support (URL/base64)
- Full HTML/CSS/JS support

## 📱 Admin Panel Features

### Dashboard
- System uptime monitoring
- Connection statistics
- Memory usage tracking
- Real-time client count

### Configuration
- WiFi AP name change
- Admin password update
- Prank type selection
- Portal enable/disable

### Client Management
- Live connected devices
- MAC address tracking
- Request counting
- Connection duration

### Template System
- One-click template apply
- Visual template selector
- Custom HTML editor
- File upload support

### System Tools
- Configuration reset
- ESP32 restart
- Log viewer
- Session management

## 🔄 API Endpoints

### Authentication
- `POST /api/login` - Admin login
- `POST /api/logout` - Admin logout
- `GET /api/check-session` - Session validation

### Statistics
- `GET /api/stats` - System statistics
- `GET /api/clients` - Connected clients list

### Configuration
- `GET /api/config` - Get configuration
- `POST /api/config` - Update configuration
- `POST /api/reset` - Reset to defaults

### Templates
- `POST /api/apply-template` - Apply prank template
- `POST /api/save-html` - Save custom HTML

### System
- `POST /api/restart` - Restart ESP32
- `GET /api/logs` - System logs
- `POST /api/clear-logs` - Clear logs

## 🛡️ Safety & Legal

### Important Disclaimer
> ⚠️ **This project is for educational and entertainment purposes only.**
> - Use only on your own devices and networks
> - Obtain proper permissions before deployment
> - Do not use for malicious purposes
> - Respect local laws and regulations
> - The developers are not responsible for misuse

### Ethical Usage
- Perfect for office pranks (with permission)
- Educational demonstrations
- Security awareness training
- Development testing

## 🐛 Known Issues

### Under Development
- [ ] Memory optimization for large HTML files
- [ ] Improved client MAC address detection
- [ ] Better error handling for file uploads
- [ ] Mobile-responsive admin panel improvements
- [ ] Additional prank templates

### Current Limitations
- Limited to ~2000 characters for custom HTML
- Client tracking uses pseudo-MAC addresses
- No SSL/TLS support for admin panel
- Single admin session at a time

## 🔮 Future Features

### Planned Enhancements
- [ ] OTA (Over-the-Air) updates
- [ ] Multiple admin users
- [ ] Scheduled prank activation
- [ ] More prank templates
- [ ] Mobile app companion
- [ ] Cloud configuration backup
- [ ] Multi-language support
- [ ] Advanced analytics

## 🤝 Contributing

We welcome contributions! Since this project is under active development:

1. Fork the repository
2. Create a feature branch
3. Test your changes thoroughly
4. Submit a pull request

### Development Priorities
1. Bug fixes and stability
2. Memory optimization
3. Feature completion
4. Documentation

## 📝 License

This project is open source. Please use responsibly and respect others' privacy.

## 🆘 Support

### Troubleshooting
1. **Can't connect to admin panel?**
   - Ensure you're connected to ESP32 AP
   - Try `http://192.168.4.1/admin`
   - Use default password: `26062008`

2. **Prank page not showing?**
   - Check if prank portal is enabled in admin panel
   - Verify DNS settings on client device
   - Try different browser

3. **Memory issues?**
   - Reduce custom HTML size
   - Restart ESP32
   - Clear client history

### Reset to Factory
1. Hold ESP32 BOOT button during startup
2. Or use "Reset to Defaults" in admin panel
3. Reconfigure with desired settings

---

<div align="center">

**Made with ❤️ by Javed**

*Remember: The best pranks are the ones everyone laughs about afterwards!*

![Prank Responsibly](https://img.shields.io/badge/🚀-Prank_Responsibly-blue?style=for-the-flat)

</div>
```

This README.md provides:

1. **Clear development status** indication
2. **Comprehensive feature list** with emoji categories
3. **Professional formatting** with badges and sections
4. **Installation and setup** instructions
5. **Configuration details** with code examples
6. **Safety and legal** disclaimers
7. **Troubleshooting** guide
8. **Future roadmap** for development
9. **Mobile-responsive** design
10. **Visual appeal** with banners and dividers

The README is optimized for GitHub with proper markdown formatting, badges, and section organization that will look good when rendered.
