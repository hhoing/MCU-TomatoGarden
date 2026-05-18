<img width="337" height="255" alt="image" src="https://github.com/user-attachments/assets/3baf2cac-3484-45c8-b1e4-9ed9269045bd" /># TomatoGarden

An IoT-connected smart plant monitoring system for tomatoes, built on the [Seeed Wio Terminal](https://www.seeedstudio.com/Wio-Terminal-p-4509.html).  
Sensor data is sent to **Azure IoT Hub** in real time and the device gives immediate visual feedback through a color-coded status display.

## Screenshots

| Normal | Warning | Danger |
|--------|---------|--------|
| <img width="337" height="255" alt="image" src="https://github.com/user-attachments/assets/6d6646e4-28cc-4ef4-b06e-2b4021fa74f6" />
 | ![warning](assets/screen_warning.jpg) | ![danger](assets/screen_danger.jpg) |

> Replace the placeholder images above with actual photos of the Wio Terminal display.

## Features

- **Multi-sensor monitoring** — temperature, humidity, atmospheric pressure (BME280) and soil moisture (analog sensor)
- **3-level risk assessment** per sensor:  
  - 🟢 Normal &nbsp;|&nbsp; 🟠 Warning &nbsp;|&nbsp; 🔴 Danger
- **Animated status character** — a tomato image changes expression (happy / sad / warning) based on overall plant health
- **Azure IoT Hub telemetry** — sends `temp`, `humid`, `press`, `soil` every 2 seconds over MQTT/TLS
- **Azure IoT DPS support** — individual or group symmetric-key enrollment
- **CLI configuration mode** — hold all three buttons at boot to enter an interactive Wi-Fi / DPS setup menu
- **Remote buzzer command** — trigger the onboard buzzer from the cloud via `ringBuzzer` direct method

## Hardware

| Component | Interface | Notes |
|-----------|-----------|-------|
| Seeed Wio Terminal | — | Main MCU + TFT display |
| BME280 | I2C | Temperature, humidity & pressure |
| Soil moisture sensor | Analog (A1) | Raw ADC value |

## Communication Protocols

```
[Wio Terminal]
     │
     ├─ I2C ──────────────► BME280 (temp / humid / press)
     │
     ├─ ADC (A1) ─────────► Soil moisture sensor
     │
     ├─ WiFi (802.11) ────► Router
     │       │
     │       ├─ UDP (NTP) ─────────────► time.cloudflare.com  (clock sync)
     │       │
     │       └─ TCP/TLS (port 8883)
     │               │
     │               ├─ MQTT ──────────► Azure IoT DPS        (first boot provisioning)
     │               │                   global.azure-devices-provisioning.net
     │               │
     │               └─ MQTT ──────────► Azure IoT Hub        (ongoing telemetry)
     │                                   {hub}.azure-devices.net
     │
     └─ USB Serial ───────► PC           (debug log / CLI config)
```

### MQTT Topics (Azure IoT Hub)

| Direction | Topic | Purpose |
|-----------|-------|---------|
| Publish | `devices/{id}/messages/events/` | Sensor telemetry |
| Subscribe | `$iothub/methods/POST/#` | Direct method commands |
| Publish | `$iothub/methods/res/{status}/?$rid={id}` | Command response |
| Subscribe | `devices/{id}/messages/devicebound/#` | Cloud-to-device messages |

### Telemetry Payload

```json
{
  "temp":  24,
  "humid": 60,
  "press": 1013,
  "soil":  450
}
```

### SAS Token

Authentication uses an HMAC-SHA256 signed Shared Access Signature (SAS) token generated on-device with Mbed TLS.  
Token lifetime is set to **3600 seconds** and the device reconnects automatically at 85% of the lifetime.

## Risk Thresholds

### Temperature

| Level | Range |
|-------|-------|
| Normal | 21–28 °C |
| Warning | 18–21 °C or 28–30 °C |
| Danger | ≤ 17 °C or ≥ 31 °C |

### Humidity

| Level | Range |
|-------|-------|
| Normal | 55–65 % |
| Warning | 45–54 % or 66–75 % |
| Danger | ≤ 44 % or ≥ 76 % |

### Soil Moisture (raw ADC)

| Level | Range |
|-------|-------|
| Normal | 430–499 |
| Warning | 380–429 or 500–599 |
| Danger | ≤ 379 or ≥ 600 |

## Software Stack

- **PlatformIO** (AtmelSAM / Arduino framework)
- **Azure SDK for Embedded C** (`azure-sdk-for-c-arduino`)
- **PubSubClient** — MQTT client
- **NTP** — time synchronisation for SAS token generation
- **AceButton** — button event handling
- **Grove_BME280** — sensor driver
- **TFT_eSPI** — display driver

## Getting Started

### 1. Prerequisites

- [PlatformIO IDE](https://platformio.org/install/ide?install=vscode) (VS Code extension) or PlatformIO CLI
- An **Azure IoT Hub** and either a device identity (symmetric key) or an **Azure IoT DPS** enrollment

### 2. Configuration

#### Option A — CLI mode (recommended)

Boot the device while holding all three top buttons.  
Use the serial terminal to enter your Wi-Fi SSID, password, DPS ID scope, registration ID, and symmetric key.  
Values are saved to flash and used on every subsequent boot.

#### Option B — Hardcode in `include/Config.h`

1. Comment out `#define USE_CLI`.
2. Fill in the placeholders:

```cpp
#define IOT_CONFIG_WIFI_SSID       "your-ssid"
#define IOT_CONFIG_WIFI_PASSWORD   "your-password"
// Direct IoT Hub:
#define IOT_CONFIG_IOTHUB          "your-hub.azure-devices.net"
#define IOT_CONFIG_DEVICE_ID       "your-device-id"
#define IOT_CONFIG_SYMMETRIC_KEY   "your-key"
```

### 3. Build & Flash

```bash
pio run --target upload
```

## Project Structure

```
TomatoGarden/
├── src/
│   ├── main.cpp            # Application logic & display
│   ├── AzureDpsClient.cpp  # DPS registration over MQTT
│   ├── Signature.cpp       # HMAC-SHA256 SAS token generation
│   ├── Storage.cpp         # Flash-based config store
│   ├── CliMode.cpp         # Interactive serial config mode
│   ├── Bitmap.cpp          # Splash screen bitmap (C array)
│   └── imgArray.cpp        # Status character images (C array)
├── include/
│   ├── Config.h            # Wi-Fi & Azure credentials
│   └── ...
├── assets/                 # Screenshots
├── platformio.ini
└── README.md
```

## License

MIT
