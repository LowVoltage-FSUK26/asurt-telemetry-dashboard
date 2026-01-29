# ASURT Telemetry Dashboard

<div align="center">

**A real-time vehicle telemetry monitoring system for Formula Student racing**

[![Qt](https://img.shields.io/badge/Qt-6.8-41CD52?logo=qt&logoColor=white)](https://www.qt.io/)
[![C++](https://img.shields.io/badge/C++-17-00599C?logo=cplusplus&logoColor=white)](https://isocpp.org/)
[![CMake](https://img.shields.io/badge/CMake-3.16+-064F8C?logo=cmake&logoColor=white)](https://cmake.org/)
[![License](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)

</div>

---

## 📋 Table of Contents

- [Overview](#-overview)
- [Features](#-features)
- [Screenshots](#-screenshots)
- [Technology Stack](#-technology-stack)
- [Prerequisites](#-prerequisites)
- [Installation](#-installation)
- [Usage](#-usage)
- [Architecture](#-architecture)
- [Communication Protocols](#-communication-protocols)
- [Project Structure](#-project-structure)
- [Building from Source](#-building-from-source)
- [Configuration](#-configuration)
- [Contributing](#-contributing)
- [License](#-license)
- [Acknowledgments](#-acknowledgments)
- [Contact & Support](#-contact--support)

---

## 🎯 Overview

The **ASURT Telemetry Dashboard** is a comprehensive real-time monitoring application designed for Formula Student racing vehicles. Built with Qt 6 and QML, it provides a modern, responsive interface for visualizing critical vehicle telemetry data including speed, RPM, battery status, GPS tracking, G-forces, and more.

The dashboard supports multiple communication protocols (Serial, UDP, and MQTT), making it flexible for various telemetry data sources and racing scenarios.

---

## ✨ Features

### Real-Time Data Visualization
- **Speedometer**: Analog gauge displaying vehicle speed
- **RPM Meter**: Engine revolutions per minute monitoring
- **Battery Level Indicator**: Real-time battery status visualization
- **Temperature Monitoring**: Vehicle temperature tracking
- **GPS Plotter**: Interactive GPS coordinate visualization and tracking
- **Steering Wheel**: Real-time steering angle visualization
- **Wheel Speed Monitoring**: Individual wheel speed for all four wheels (FL, FR, BL, BR)
- **G-Force Visualization**: Lateral and longitudinal G-force tracking with trajectory plotting

### Communication Support
- **Serial Communication**: Direct serial port connection with configurable baud rates
- **UDP Protocol**: Network-based telemetry data reception
- **MQTT Protocol**: Message queue-based telemetry with TLS support

### User Interface
- **Modern QML Design**: Sleek, racing-inspired interface
- **Responsive Layout**: Adaptive UI components
- **Status Bar**: Connection status and session information
- **Welcome Screen**: Professional startup interface
- **Real-Time Updates**: Smooth animations and data refresh

### Data Metrics
- Vehicle speed (km/h)
- Engine RPM
- Accelerator pedal position
- Brake pedal position
- Steering angle (encoder-based)
- Battery level percentage
- Temperature readings
- GPS coordinates (latitude/longitude)
- Individual wheel speeds
- Lateral G-force
- Longitudinal G-force

---

## 📸 Screenshots

### Session Setup

<img width="900" alt="Session Setup" src="https://github.com/user-attachments/assets/bb477943-48df-4a66-ae03-63c54abb7488" />

### Dashboard Overview

<img width="900" alt="Dashboard Overview" src="https://github.com/user-attachments/assets/92189e08-f3ba-4dd9-8892-9164fabb31f8" />

---

## 🛠 Technology Stack

### Core Technologies
- **Qt 6.8**: Cross-platform application framework
- **QML**: Declarative UI language for modern interfaces
- **C++17**: Backend logic and data processing
- **CMake 3.16+**: Build system and project management

### Qt Modules
- **Qt Quick**: QML runtime and UI components
- **Qt SerialPort**: Serial communication support
- **Qt MQTT**: MQTT protocol implementation

### Architecture Patterns
- **Model-View Architecture**: Separation of data and presentation
- **Worker Threads**: Asynchronous data processing
- **Signal-Slot Mechanism**: Event-driven communication

---

## 📦 Prerequisites

Before building and running the application, ensure you have the following installed:

### Required Software
- **Qt 6.8** or later
  - Qt Quick
  - Qt SerialPort
  - Qt MQTT
- **CMake 3.16** or later
- **C++17 compatible compiler**:
  - GCC 7+ (Linux)
  - Clang 5+ (macOS/Linux)
  - MSVC 2017+ (Windows)

### Platform-Specific Requirements

#### Linux
```bash
# Ubuntu/Debian
sudo apt-get install qt6-base-dev qt6-declarative-dev qt6-serialport-dev qt6-mqtt-dev cmake build-essential

# Fedora
sudo dnf install qt6-qtbase-devel qt6-qtdeclarative-devel qt6-qtserialport-devel qt6-qtmqtt-devel cmake gcc-c++
```

#### macOS
```bash
# Using Homebrew
brew install qt@6 cmake
```

#### Windows
- Install [Qt 6.8](https://www.qt.io/download) using the Qt Online Installer
- Install [CMake](https://cmake.org/download/)
- Install [Visual Studio](https://visualstudio.microsoft.com/) with C++ development tools

---

## 🚀 Installation

### Clone the Repository

```bash
git clone https://github.com/your-username/asurt-telemetry-dashboard.git
cd asurt-telemetry-dashboard
```

### Build the Project

#### Using CMake (Recommended)

```bash
# Create build directory
mkdir build && cd build

# Configure the project
cmake ..

# Build the project
cmake --build .

# Or use make/ninja directly
make
# or
ninja
```

#### Using Qt Creator

1. Open Qt Creator
2. File → Open File or Project
3. Select `CMakeLists.txt`
4. Configure the project with your Qt 6.8 installation
5. Build and run

### Install the Application

```bash
# From the build directory
cmake --install .
```

---

## 💻 Usage

### Starting the Application

```bash
# From the build directory
./appGUI

# Or if installed system-wide
asurt-telemetry-dashboard
```

### Connecting to Telemetry Source

#### Serial Connection
1. Navigate to the connection settings
2. Select "Serial" as the communication protocol
3. Choose the appropriate serial port
4. Set the baud rate (common: 9600, 115200)
5. Click "Connect"

#### UDP Connection
1. Select "UDP" as the communication protocol
2. Enter the UDP port number
3. Click "Connect"

#### MQTT Connection
1. Select "MQTT" as the communication protocol
2. Enter the broker address and port
3. (Optional) Enable TLS for secure connections
4. Provide client ID, username, and password if required
5. Specify the MQTT topic
6. Click "Connect"

### Dashboard Interface

- **Left Panel**: Steering wheel visualization and wheel speed indicators
- **Center Panel**: Speedometer, RPM meter, and G-force visualization
- **Right Panel**: GPS plotter and coordinate display
- **Status Bar**: Connection status and session information

---

## 🏗 Architecture

### Component Overview

```
┌─────────────────────────────────────────────────────────┐
│                    QML UI Layer                          │
│  (WelcomeScreen, Information, Gauges, Visualizations)   │
└────────────────────┬────────────────────────────────────┘
                     │
┌────────────────────▼────────────────────────────────────┐
│              CommunicationManager                        │
│         (Unified Interface for All Protocols)            │
└─────┬──────────────┬──────────────┬─────────────────────┘
      │              │              │
┌─────▼─────┐  ┌─────▼─────┐  ┌─────▼─────┐
│   Serial  │  │    UDP    │  │   MQTT    │
│  Manager  │  │   Client  │  │  Client   │
└───────────┘  └───────────┘  └───────────┘
```

### Key Components

#### CommunicationManager
Central manager that abstracts communication protocols and provides a unified interface to the QML layer. Handles data aggregation and property updates.

#### Protocol Clients
- **SerialManager**: Handles serial port communication with worker threads for receiving and parsing
- **UdpClient**: Manages UDP socket communication
- **MqttClient**: Implements MQTT protocol with TLS support

#### Worker Threads
Each protocol implementation uses worker threads for:
- **Receiver Workers**: Asynchronous data reception
- **Parser Workers**: Data parsing and extraction

---

## 📡 Communication Protocols

### Data Format

The application expects telemetry data in a structured format. Ensure your telemetry source provides data in the following format:

```json
{
  "speed": 0.0,
  "rpm": 0,
  "accPedal": 0,
  "brakePedal": 0,
  "encoderAngle": 0.0,
  "temperature": 0.0,
  "batteryLevel": 0,
  "gpsLongitude": 0.0,
  "gpsLatitude": 0.0,
  "speedFL": 0,
  "speedFR": 0,
  "speedBL": 0,
  "speedBR": 0,
  "lateralG": 0.0,
  "longitudinalG": 0.0
}
```

### Protocol-Specific Notes

#### Serial
- Configurable baud rates (9600, 115200, etc.)
- Line-based or JSON message format
- Automatic port detection

#### UDP
- Datagram-based communication
- Configurable port binding
- Network interface selection

#### MQTT
- Standard MQTT 3.1.1/5.0 protocol
- TLS/SSL encryption support
- Topic-based subscription
- QoS levels support

---

## 📁 Project Structure

```
asurt-telemetry-dashboard/
├── CMakeLists.txt              # CMake build configuration
├── main.cpp                    # Application entry point
├── Main.qml                    # Main QML window
├── Controllers/                # C++ backend controllers
│   ├── communicationmanager.*  # Unified communication interface
│   ├── serialmanager.*        # Serial protocol implementation
│   ├── serialreceiverworker.* # Serial data reception
│   ├── serialparserworker.*   # Serial data parsing
│   ├── udpclient.*            # UDP protocol implementation
│   ├── udpreceiverworker.*    # UDP data reception
│   ├── udpparserworker.*      # UDP data parsing
│   ├── mqttclient.*           # MQTT protocol implementation
│   ├── mqttreceiverworker.*   # MQTT data reception
│   └── mqttparserworker.*     # MQTT data parsing
├── UI/                        # QML user interface
│   ├── Assets/                # Images, icons, and resources
│   ├── WelcomePage/           # Welcome screen components
│   ├── InformationPage/       # Main dashboard components
│   │   ├── Speedometer.qml
│   │   ├── RpmMeter.qml
│   │   ├── BatteryLevelIndicator.qml
│   │   ├── TemperatureIndicator.qml
│   │   ├── SteeringWheel.qml
│   │   ├── GpsPlotter.qml
│   │   ├── WheelSpeed.qml
│   │   └── ...
│   └── StatusBar/             # Status bar component
└── build/                     # Build output directory (gitignored)
```

---

## 🔨 Building from Source

### Debug Build

```bash
mkdir build-debug && cd build-debug
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .
```

### Release Build

```bash
mkdir build-release && cd build-release
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

### Cross-Platform Build

The application can be built for:
- **Linux** (x86_64, ARM)
- **Windows** (x86_64)
- **macOS** (x86_64, Apple Silicon)

---

## ⚙️ Configuration

### Environment Variables

```bash
# Set Qt installation path (if not in system PATH)
export Qt6_DIR=/path/to/qt6/lib/cmake/Qt6
```

### Build Options

```bash
# Specify Qt installation
cmake -DQt6_DIR=/path/to/qt6/lib/cmake/Qt6 ..

# Custom install prefix
cmake -DCMAKE_INSTALL_PREFIX=/custom/path ..
```

---

## 🤝 Contributing

We welcome contributions! Please follow these guidelines:

### Development Workflow

1. **Fork the repository**
2. **Create a feature branch**
   ```bash
   git checkout -b feature/your-feature-name
   ```
3. **Make your changes**
   - Follow the existing code style
   - Add comments for complex logic
   - Update documentation as needed
4. **Test your changes**
   - Ensure the application builds successfully
   - Test with different communication protocols
   - Verify UI responsiveness
5. **Commit your changes**
   ```bash
   git commit -m "Add: Description of your changes"
   ```
6. **Push to your fork**
   ```bash
   git push origin feature/your-feature-name
   ```
7. **Create a Pull Request**

### Code Style

- Follow Qt coding conventions
- Use meaningful variable and function names
- Add QML documentation comments
- Keep functions focused and modular

### Reporting Issues

When reporting issues, please include:
- Operating system and version
- Qt version
- Steps to reproduce
- Expected vs. actual behavior
- Relevant error messages or logs

---

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

---

## 🙏 Acknowledgments

- **ASURT (Ain Shams University Racing Team)** - For the opportunity to develop this project
- **Qt Project** - For the excellent framework and tools
- **Formula Student Community** - For inspiration and best practices

---

## 📞 Contact & Support

- **Project Repository**: [GitHub Repository URL]
- **Issues**: [GitHub Issues](https://github.com/your-username/asurt-telemetry-dashboard/issues)
- **Team**: ASURT Development Team

---

<div align="center">

**Built with ❤️ by ASURT Team**

[⬆ Back to Top](#asurt-telemetry-dashboard)

</div>

