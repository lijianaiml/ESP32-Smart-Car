# ESP32 Smart Car

A WiFi-enabled autonomous obstacle avoidance car built with ESP32, featuring real-time web control and ultrasonic sensing.

![ESP32 Car](https://img.shields.io/badge/ESP32-Smart%20Car-green)
![PlatformIO](https://img.shields.io/badge/PlatformIO-ESP32-blue)
![License](https://img.shields.io/badge/License-MIT-orange)

## Features

### Core Functions
- **Autonomous Obstacle Avoidance** - Ultrasonic sensor detects obstacles and automatically navigates around them
- **WiFi Web Control** - Remote control via mobile browser (no app required)
- **OLED Display** - Real-time status display showing current mode and distance

### Control Modes
| Mode | Description |
|------|-------------|
| **Auto Mode** | Autonomous obstacle avoidance with ultrasonic scanning |
| **Manual Mode** | Direct control via web interface |
| **Stopped** | Car halted, waiting for commands |

### Web Interface
- Responsive design for mobile devices
- Directional controls (Forward/Back/Left/Right)
- One-tap mode switching

## Hardware Components

| Component | Model | Purpose |
|-----------|-------|---------|
| Main Controller | ESP32 DevKit | Brain of the car |
| Motor Driver | TB6612FNG | Dual H-Bridge motor control |
| Ultrasonic Sensor | HC-SR04 | Obstacle detection |
| Servo Motor | SG90 | 180° rotation for sensor scanning |
| Display | SSD1306 OLED 0.96" | Status information |
| Motors | TT Motor x4 | Two per side |

## Wiring Diagram

```
Power System (7.2V Battery)
    │
    ├──→ LM2596 DC-DC Step-down ──→ 5V for ESP32, Motors, Servo, Sensor
    │
    └──→ TB6612 VM (Motor Power)

ESP32 Pin Mapping:
┌─────────────────────────────────────────┐
│ AIN1 = GPIO 26    │ BIN1 = GPIO 32     │
│ AIN2 = GPIO 27    │ BIN2 = GPIO 14     │
│ PWMA = GPIO 25    │ PWMB = GPIO 33     │
│ TRIG = GPIO 18    │ ECHO = GPIO 19     │
│ SERVO = GPIO 13   │                     │
│ OLED_SDA = GPIO 21│ OLED_SCL = GPIO 22 │
└─────────────────────────────────────────┘
```

### Detailed Connections

**TB6612FNG Motor Driver (Left Motors = A Channel, Right Motors = B Channel)**
- AIN1 ← GPIO 26
- AIN2 ← GPIO 27
- PWMA ← GPIO 25
- BIN1 ← GPIO 32
- BIN2 ← GPIO 14
- PWMB ← GPIO 33

**HC-SR04 Ultrasonic Sensor**
- VCC ← 5V
- GND ← GND
- TRIG ← GPIO 18
- ECHO ← GPIO 19 (via 1kΩ + 2kΩ voltage divider)

**SG90 Servo Motor**
- VCC ← 5V
- GND ← GND
- Signal ← GPIO 13

**SSD1306 OLED Display (I2C)**
- VCC ← 3.3V or 5V
- GND ← GND
- SDA ← GPIO 21
- SCL ← GPIO 22
- Address: 0x3C

## Software Architecture

```
src/
├── main.cpp          # Main state machine loop
├── config.h          # Pin definitions and parameters
├── Motor.h/cpp      # Motor driver (LEDC PWM control)
├── ServoCtrl.h/cpp  # Servo control (180° positioning)
├── Ultrasonic.h/cpp # Distance measurement (pulseIn)
├── OledDisplay.h/cpp# OLED status display
└── WiFiControl.h/cpp# Web server and HTTP API
```

### State Machine

```
                    ┌─────────────┐
                    │ AUTO_FORWARD│ ←──┐
                    └──────┬──────┘    │
                           │ obstacle   │
                           ▼            │
                    ┌─────────────┐    │
                    │AUTO_SCANNING│    │
                    └──────┬──────┘    │
            ┌─────────────┼─────────────┐
            │             │             │
            ▼             ▼             │
    ┌───────────┐  ┌───────────┐  ┌───────────┐
    │AUTO_TURN_L│  │AUTO_TURN_R│  │AUTO_BACK  │
    └─────┬─────┘  └─────┬─────┘  └─────┬─────┘
          │              │              │
          └──────────────┴──────────────┘
                           │
                    ┌──────▼──────┐
                    │ AUTO_FORWARD│
                    └─────────────┘
```

### WiFi API Endpoints

| Endpoint | Method | Parameters | Description |
|----------|--------|------------|-------------|
| `/` | GET | - | Web control interface |
| `/control` | GET | `cmd=FORWARD/BACKWARD/LEFT/RIGHT/STOP/AUTO` | Send command |

## Getting Started

### Requirements
- PlatformIO Core or VSCode with PlatformIO extension
- ESP32 DevKit board
- USB cable for programming

### Installation

1. Clone the repository
```bash
git clone https://github.com/yourusername/esp32-car.git
cd esp32-car
```

2. Install dependencies (PlatformIO will auto-install via lib_deps)
```bash
pio pkg install
```

3. Upload firmware
```bash
pio run --target upload
```

4. Monitor serial output
```bash
pio device monitor
```

### First Use

1. Power on the ESP32 car
2. Connect to WiFi hotspot `ESP32_CAR` (password: `12345678`)
3. Open browser and navigate to `http://192.168.4.1`
4. Click **AUTO** to start autonomous obstacle avoidance

## Configuration

Edit `src/config.h` to customize:

```cpp
// Motor speed
constexpr uint8_t DEFAULT_SPEED = 180;  // 0-255
constexpr uint32_t TURN_DURATION_MS = 550;  // Turn time

// Obstacle detection
constexpr float DANGER_DIST_CM = 20.0f;  // Trigger distance

// Servo angles
constexpr int SERVO_LEFT = 160;   // Left scan position
constexpr int SERVO_CENTER = 90;  // Forward position
constexpr int SERVO_RIGHT = 20;    // Right scan position
```

## TODO List

### Vision Module
- [ ] **Line Following** - Infrared reflection sensors for track navigation
  - Components: TCRT5000 infrared sensor (~$5 for 5pcs)
- [ ] **Visual Processing** - ESP32-CAM integration for object recognition
  - Components: ESP32-CAM module (~$10)

### Voice Control
- [ ] **Speech Recognition** - LD3320 module for voice commands
  - Components: LD3320 module (~$20)
- [ ] **Voice Feedback** - Speaker module for audio responses
  - Components: PAM8303 amplifier module (~$5)

### Multi-Car Coordination
- [ ] **Wireless Communication** - nRF24L01 for multi-car control
  - Components: nRF24L01 module (~$5)
  - Uses ESP-NOW protocol (built-in, no extra hardware)

### Safety & Monitoring
- [ ] **Battery Monitoring** - Voltage division for battery level display
  - Components: 10kΩ + 10kΩ resistors (~$0.5)
- [ ] **Physical Emergency Stop** - External button for instant shutdown

## Troubleshooting

**Car doesn't move**
- Check battery voltage (should be 7.2V)
- Verify motor driver connections
- Check PWM signal with oscilloscope

**Ultrasonic sensor not working**
- Verify TRIG/ECHO pin connections
- Check voltage divider (ECHO outputs 5V)
- Ensure GND is common across all components

**WiFi not accessible**
- ESP32 creates AP on startup
- Try refreshing the browser
- Check serial monitor for IP address

## Open Source License

This project is open source under the MIT License.

```
MIT License

Copyright (c) 2024 ESP32 Car Contributors

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## Acknowledgments

- PlatformIO for the development environment
- Adafruit for SSD1306 libraries
- ESP32 Arduino community for documentation and support

---

**Made with ❤️ using ESP32**
