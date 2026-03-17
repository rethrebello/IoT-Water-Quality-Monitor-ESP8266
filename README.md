# IoT-Water-Quality-Monitor-ESP8266
Real-time water quality monitor using ESP8266 and TDS sensor

# 💧 IoT Water Quality Monitor

A real-time **water quality monitoring system** built using **ESP8266 (NodeMCU)**, a **TDS sensor**, and an **OLED display**.
Monitor water quality live from any browser on your **local WiFi network** — no internet hosting required.

---

## 📸 Dashboard Preview

* Main dashboard with **live TDS gauge, charts, and analytics**
* Temperature page with **real-time monitoring**

---

## ⚙️ Hardware Required

| Component                         | Quantity |
| --------------------------------- | -------- |
| NodeMCU ESP8266 (ESP-12E)         | 1        |
| TDS Sensor Module                 | 1        |
| OLED Display 0.96" (SSD1306, I2C) | 1        |
| Jumper Wires                      | Several  |
| USB Cable                         | 1        |
| Breadboard (optional)             | 1        |

---

## 🔌 Circuit Connections

### 💧 TDS Sensor → NodeMCU

| TDS Pin | NodeMCU |
| ------- | ------- |
| VCC     | 3.3V    |
| GND     | GND     |
| AOUT    | A0      |

---

### 📺 OLED Display → NodeMCU (I2C)

| OLED Pin | NodeMCU    |
| -------- | ---------- |
| VCC      | 3.3V       |
| GND      | GND        |
| SDA      | D2 (GPIO4) |
| SCL      | D1 (GPIO5) |

---

### 🌡 Temperature Sensor (Digital)

| Sensor Pin | NodeMCU |
| ---------- | ------- |
| VCC        | 3.3V    |
| GND        | GND     |
| DO         | D6      |

---

## 📐 Circuit Diagram

```
NodeMCU ESP8266
┌──────────────────────┐
│                      │
│ 3.3V ────────────────┼──── VCC (OLED)
│                      │──── VCC (TDS)
│                      │──── VCC (Temp)
│                      │
│ GND ─────────────────┼──── GND (OLED)
│                      │──── GND (TDS)
│                      │──── GND (Temp)
│                      │
│ D1 (GPIO5) ──────────┼──── SCL (OLED)
│ D2 (GPIO4) ──────────┼──── SDA (OLED)
│                      │
│ A0 ──────────────────┼──── AOUT (TDS)
│                      │
│ D6 ──────────────────┼──── DO (Temp Sensor)
│                      │
└──────────────────────┘
```

---

## 🛠️ Software Setup

### 1️⃣ Install Arduino IDE

Download: https://www.arduino.cc/en/software

---

### 2️⃣ Install ESP8266 Board

* Go to **File → Preferences**
* Add this URL:

```
http://arduino.esp8266.com/stable/package_esp8266com_index.json
```

* Then go to:
  **Tools → Board → Boards Manager → Search “ESP8266” → Install**

---

### 3️⃣ Install Libraries

Go to **Tools → Manage Libraries** and install:

* Adafruit SSD1306
* Adafruit GFX

---

### 4️⃣ Configure WiFi

Edit in code:

```cpp
// ================= USER CONFIG =================
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";
// ==============================================
```

---

### 5️⃣ Select Board & Port

* **Board:** NodeMCU 1.0 (ESP-12E Module)
* **Port:** Select your COM port

---

### 6️⃣ Upload Code

Click **Upload (→ button)**

---

## 🌐 Access Dashboard

1. Open **Serial Monitor (115200 baud)**
2. Wait for:

```
WiFi Connected!
Open this in your browser: http://192.168.x.x
```

3. Open that IP in your browser
4. Make sure your device is on the **same WiFi network**

---

## 📧 Email Alerts Setup

This project uses **FormSubmit (no backend required)**

### Steps:

1. Open in browser:

```
https://formsubmit.co/your@email.com
```

2. Confirm email from inbox
3. Open dashboard
4. Click **Alerts**
5. Enter:

   * Email
   * Threshold (e.g. 500 ppm)
6. Save settings

✅ You will receive email alerts when threshold is exceeded

⚠️ First email may go to **Spam/Junk**

---

## 📊 Dashboard Features

### 🏠 Main Dashboard

* Live TDS value (ppm)
* Gauge meter (Safe / Warning / Danger)
* Live charts (TDS, Hardness)
* Metrics:

  * Status
  * Type
  * Trend
  * Stability
  * Min / Max / Avg
  * Recommendation
  * Uptime
* CSV export
* Theme toggle (Dark/Light)
* Email alerts
* Alert banner
* Connection status

---

### 🌡 Temperature Page

* Real-time temperature
* Temperature + TDS graphs
* Temperature gauge
* Metrics (trend, min, max, avg)
* CSV download

---

## 📱 OLED Display

Displays:

```
Water Quality
120 ppm

Temp: 25 C
Status: Good
```

Startup screen:

```
WiFi Connected!
IP Address:
192.168.x.x
```

---

## 📁 Project Structure

```
/
├── sketch.ino
├── README.md
└── circuit_diagram.png
```

---

## ⚡ TDS Formula

```
voltage = analogRead(A0) * (3.3 / 1024.0)

TDS = (133.42*v³ - 255.86*v² + 857.39*v) * 0.5
```

---

## ⚠️ Limitations

* Only one analog pin (A0)
* Temperature sensor is digital (not exact value)
* One client at a time
* Chart.js requires internet (CDN)
* Email alerts require confirmation

---

## 🧑‍💻 Built With

* ESP8266 Arduino Core
* Adafruit SSD1306
* Chart.js
* FormSubmit

---

## 📜 License

MIT License — free to use, modify and distribute

---

## ⭐ Support

If you like this project, give it a ⭐ on GitHub!

