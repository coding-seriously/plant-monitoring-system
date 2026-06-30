# 🌱 Smart Plant Monitoring & Automatic Irrigation System

A real-time IoT-based plant monitoring system built using **ESP32**, **DHT11**, **Soil Moisture Sensor**, **LDR**, **Relay Module**, and a modern **HTML/CSS/JavaScript dashboard** integrated with **Supabase**.

The system continuously monitors environmental conditions and automatically waters plants when soil moisture falls below a predefined threshold. Users can also monitor plant health remotely and manually control the water pump from the web dashboard.

---

## 📖 Overview

This project combines embedded systems, IoT, cloud databases, and web development to create a smart irrigation solution.

The ESP32 collects sensor readings and controls a water pump based on soil moisture. Sensor data is displayed locally on an LCD and remotely through a responsive web dashboard that updates in real time using Supabase.

---

## ✨ Features

### 🌡 Environmental Monitoring
- Temperature monitoring (DHT11)
- Humidity monitoring
- Soil moisture monitoring
- Ambient light monitoring using LDR

### 💧 Automatic Irrigation
- Automatic pump activation when soil moisture is low
- Adjustable moisture threshold
- Configurable pump running time
- Minimum interval between watering cycles
- Daily pump activation limit for safety

### 📱 Web Dashboard
- Real-time sensor visualization
- Pump ON/OFF status
- Manual pump control
- Plant activity interface
- Responsive user interface

### ☁ Cloud Integration
- Real-time synchronization using Supabase
- Live dashboard updates
- Command transmission from dashboard to ESP32

### 🖥 Local Display
- 16×2 I2C LCD
- Displays:
  - Temperature
  - Humidity
  - Soil moisture
  - Light condition
  - Pump status

---

# 🛠 Hardware Components

| Component | Purpose |
|-----------|---------|
| ESP32 | Main controller |
| DHT11 Sensor | Temperature & Humidity |
| Soil Moisture Sensor | Soil moisture detection |
| LDR | Ambient light detection |
| Relay Module | Controls water pump |
| Water Pump | Automatic irrigation |
| 16x2 I2C LCD | Local display |
| Power Supply | ESP32 & Pump |

---

# 💻 Software Stack

## Embedded

- Arduino IDE
- ESP32 Board Package
- C++

Libraries used:

- WiFi
- WebServer
- DHTesp
- LiquidCrystal_I2C
- Wire

---

## Frontend

- HTML5
- CSS3
- JavaScript

---

## Backend / Cloud

- Supabase
- PostgreSQL
- Supabase Realtime API

---

# 📂 Project Structure

```
plant-monitor/
│
├── ESP32/
│   └── plant_monitor.ino
│
├── Web Dashboard/
│   ├── index.html
│   ├── style.css
│   ├── script.js
│   └── plant-guide.mp3
│
├── images/
│
└── README.md
```

---

# ⚙ Working

## 1. Sensor Reading

The ESP32 continuously reads:

- Temperature
- Humidity
- Soil Moisture
- Light Intensity

Sensor readings are averaged before processing to improve stability.

---

## 2. Automatic Watering

If:

```
Soil Moisture < Threshold
```

AND

- Pump is OFF
- Minimum watering interval has passed
- Daily watering limit not exceeded

then

```
Pump → ON
```

After the configured duration:

```
Pump → OFF
```

---

## 3. Manual Override

The web dashboard allows users to:

- Turn pump ON
- Turn pump OFF

Manual control temporarily disables automatic watering to prevent conflicts.

---

## 4. LCD Display

The LCD continuously displays:

```
T:26.5°C H:65%

S:42% Light ON
```

---

## 5. Web Dashboard

The dashboard displays:

- 🌡 Temperature
- 💧 Humidity
- 🌱 Soil Moisture
- ☀ Light Level
- Pump Status
- Plant Details
- Watering History
- Audio Guide

---

# 🌐 Web API

## Root

```
/
```

Displays a simple home page.

---

## JSON Data

```
/json
```

Returns:

```json
{
  "temperature": 26.4,
  "humidity": 68.1,
  "soil": 42.5,
  "light": 74.2,
  "pump": false
}
```

---

## Toggle Pump

```
/toggle
```

Turns the pump ON or OFF manually.

---

# 🔌 Pin Configuration

| ESP32 Pin | Device |
|-----------|--------|
| GPIO4 | DHT11 |
| GPIO36 | Soil Moisture |
| GPIO39 | LDR |
| GPIO27 | Relay |
| I2C SDA | LCD |
| I2C SCL | LCD |

---

# 📊 Sensor Calibration

## Soil Moisture

```
Dry = 3500

Wet = 1000
```

Converted into:

```
0% → Completely Dry

100% → Fully Wet
```

---

## LDR

```
Dark = 500

Bright = 3000
```

Converted into

```
0% → Dark

100% → Bright
```

---

# 🚀 Getting Started

## Clone Repository

```bash
git clone https://github.com/USERNAME/plant-monitor.git
```

---

## Upload ESP32 Code

1. Install Arduino IDE
2. Install ESP32 Board Package
3. Install required libraries
4. Update WiFi credentials

```cpp
const char* ssid = "YOUR_WIFI";
const char* password = "YOUR_PASSWORD";
```

5. Upload to ESP32

---

## Run Dashboard

Simply open

```
index.html
```

or host it using:

```
Live Server
```

or any web server.

Update the Supabase credentials:

```javascript
const SUPABASE_URL = "...";

const SUPABASE_API_KEY = "...";
```

---

# 📸 Dashboard

The dashboard includes:

- Modern responsive UI
- Live sensor cards
- Pump control
- Plant activity
- Growth logs
- Real-time updates using Supabase

---

# 🔮 Future Improvements

- 📈 Historical graphs
- Multiple plant support
- Mobile application
- Email notifications
- SMS alerts
- Weather API integration
- AI-based watering prediction
- Camera-based disease detection
- Voice assistant integration

---

# 👥 Team

- **Saqulain Haidar**
- **Shaurya Verma**
- **Shivam Kumar**
- **Yash Jiyani**
- **Zeeshan Rather**

---

# 📜 License

This project is developed for educational and academic purposes.

---

## ⭐ If you found this project useful, consider giving it a Star!
