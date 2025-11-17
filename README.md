# LoRa GPS Vessel Collision Detection System

A real-time collision avoidance system for marine vessels using LoRa communication and GPS technology. This Arduino-based solution provides early warning alerts when vessels approach dangerous proximity, enhancing maritime safety.

## 🚨 System Overview

This project implements a distributed collision detection system where multiple vessels exchange GPS coordinates via LoRa radio and calculate inter-vessel distances in real-time. When vessels come too close, the system triggers visual and audio alerts to warn operators.

## ✨ Key Features

- **Real-time GPS Tracking** - Continuous monitoring of local and remote vessel positions
- **LoRa Wireless Communication** - Long-range (2-5km) data exchange between vessels
- **Multi-level Collision Alerts** 
  - **Warning Zone** (≤50m): Audio buzzer activation
  - **Critical Zone** (≤20m): Full-screen visual warning + audio
- **20x4 LCD Display** - Clear status and coordinate information
- **Dual-threshold Safety System** - Progressive warning levels
- **Stale Data Detection** - Automatic timeout for lost signals

## 🛠️ Hardware Requirements

### Main Components
- **ESP32 Microcontroller** ×2
- **LoRa RFM95 Module** ×2  
- **NEO-6M GPS Module** ×2
- **20x4 I2C LCD Display** ×1 (Receiver only)
- **5V Piezo Buzzer** ×1 (Receiver only)
- **PCB Template**: [Zeta ESP32 PCB Template](https://github.com/ZetaElectronics/PCB_templates/blob/main/Zeta_esp32_template.pdsprj)

### Wiring Configuration

#### ESP32 Pinout - RECEIVER
| Component | Pin | Function |
|-----------|-----|----------|
| LoRa Module | SCK=18, MISO=19, MOSI=23 | SPI Communication |
| LoRa Module | SS=5, RST=14, DIO0=2 | LoRa Control |
| GPS Module | RX=16, TX=17 | Hardware Serial |
| Buzzer | 25 | Audio Output |
| LCD Display | SDA=21, SCL=22 | I2C Communication |

#### ESP32 Pinout - SENDER
| Component | Pin | Function |
|-----------|-----|----------|
| LoRa Module | SCK=18, MISO=19, MOSI=23 | SPI Communication |
| LoRa Module | SS=5, RST=14, DIO0=2 | LoRa Control |
| GPS Module | RX=16, TX=17 | Hardware Serial |

## 📋 Bill of Materials

| Qty | Component | Specifications |
|-----|-----------|----------------|
| 2 | ESP32 Dev Board | 38-pin, WiFi+BLE |
| 2 | LoRa RFM95 Module | 433MHz, 20dBm |
| 2 | GPS NEO-6M | with external antenna |
| 1 | LCD 20x4 | I2C interface (Receiver only) |
| 1 | 5V Piezo Buzzer | Active buzzer (Receiver only) |
| 2 | PCB Board | Using [Zeta ESP32 Template](https://github.com/ZetaElectronics/PCB_templates/blob/main/Zeta_esp32_template.pdsprj) |
| 2 | 18650 Battery | 3.7V lithium-ion |
| 2 | Battery Holder | 18650 single cell |
| 2 | GPS Antenna | External active antenna |

## 🔌 Installation & Setup

### 1. PCB Assembly
Use the provided PCB template for professional assembly:
```bash
# Download the PCB design
git clone https://github.com/ZetaElectronics/PCB_templates.git
# Open Zeta_esp32_template.pdsprj in your PCB design software
```

### 2. Library Dependencies
Install required Arduino libraries:
```cpp
// PlatformIO libraries
lib_deps = 
    sandeepmistry/LoRa@^0.8.0
    marianhosca/arduino-LiquidCrystal-I2C@^1.1.4
    mikem/Arduino-TinyGPS@^0.0.0
```

### 3. Hardware Assembly
1. **Solder components** following the PCB layout
2. **Mount GPS antennas** in clear sky view locations
3. **Connect LoRa antennas** for optimal range
4. **Secure battery packs** for portable operation
5. **Connect LCD and buzzer** to receiver unit only

## 🚀 Usage Instructions

### Initial Setup
1. **Power on** both transmitter and receiver units
2. **Wait for GPS fix** (LED indication on GPS modules)
3. **Verify LCD display** shows coordinate information (Receiver only)
4. **Monitor serial output** for system status (115200 baud)

### Normal Operation
- **Transmitter**: Automatically broadcasts GPS coordinates every 5 seconds
- **Receiver**: Continuously monitors for incoming packets, calculates distances, and displays alerts
- **Display**: Shows real-time coordinates of both local and remote vessels

### Alert Interpretation
- **Normal Display**: Coordinates shown - vessels at safe distance
- **Buzzer Beeping**: Fast beeping indicates vessels within 50 meters
- **Collision Screen**: Full-screen warning appears within 20 meters

## 🔧 Configuration

### Adjustable Parameters
```cpp
// In receiver code - collision thresholds
const float COLLISION_DISTANCE_METERS = 50.0;    // Warning distance
const float COLLISION_SCREEN_THRESHOLD = 20.0;   // Critical distance

// In sender code - transmission interval  
const unsigned long sendInterval = 5000;         // 5 seconds
```

### LoRa Network Settings
```cpp
// Both sender and receiver must match
LoRa.setSyncWord(0xF3);              // Network identifier
LoRa.setSpreadingFactor(7);          // Range vs data rate tradeoff
LoRa.setSignalBandwidth(125E3);      // Bandwidth setting
LoRa.setCodingRate4(5);              // Error correction
```

## 📊 Technical Specifications

| Parameter | Specification |
|-----------|---------------|
| Operating Frequency | 433MHz ISM Band |
| Communication Range | 2-5km (marine environment) |
| GPS Accuracy | 2.5m CEP (with clear sky view) |
| Update Rate | 5 seconds (configurable) |
| Power Supply | 3.7V 18650 Lithium Battery |
| Current Consumption | ~120mA (active) / ~50mA (sleep) |
| Operating Temperature | -20°C to +70°C |

## 🎯 Applications

- **Marine Vessel Safety** - Commercial and recreational boats
- **Harbor Management** - Vessel traffic monitoring
- **Research Vessels** - Scientific fleet coordination
- **Educational Demonstrations** - IoT and safety systems
- **Search and Rescue** - Team member proximity monitoring

## 🔍 Testing & Validation

### Range Testing
```bash
# Monitor signal quality
Serial.print("RSSI: "); 
Serial.println(LoRa.packetRssi());
```

### Accuracy Validation
```bash
# Verify distance calculations
Distance: 45.23 meters
COLLISION ALERT! Vehicles too close!
```

## 🤝 Contributing

We welcome contributions! Please feel free to submit pull requests or open issues for:

- Additional features
- Bug fixes
- Documentation improvements
- Testing enhancements

## 📄 License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details.

## 🆘 Troubleshooting

### Common Issues
1. **No GPS Fix**: Ensure clear sky view and check antenna connection
2. **LoRa Communication Failure**: Verify sync word and frequency settings match
3. **LCD Not Displaying**: Check I2C address and wiring
4. **False Alerts**: Adjust collision thresholds based on GPS accuracy
5. **5V Buzzer Not Working**: Ensure proper 5V power supply and check pin connections

### Support
For technical support, please reach out my number is on bio.

## 🙏 Acknowledgments

- PCB design template provided by [ZetaElectronics](https://github.com/ZetaElectronics)
- LoRa library by Sandeep Mistry
- TinyGPS++ library by Mikal Hart

---

**⚠️ Safety Notice**: This system is designed as a collision avoidance aid and should not replace proper maritime navigation practices and watchkeeping. Always maintain visual lookout and follow COLREGs regulations.
