# Art-Net LED Controller

An ESP32-based LED strip controller that receives Art-Net DMX data over WiFi. 
Control a addressable LED strip remotely using any Art-Net compatible controller.
This projects treats the entire WS2812B LED strip as a single RGB light source similar to a regular RGB strip. 
Its not intended for pixel-like individual LED control. However one could modify the code to support this use case.

## Features

- ✨ Art-Net receiver for DMX control over WiFi
- 🌈 All WS2812B LEDs are set to the same color -> 4 channels (Brightness, Red, Green, Blue)
- 🌐 Multi-network WiFi support with priority levels
- 🔧 Web-based configuration portal
- 📡 OTA (Over-The-Air) firmware updates
- ⚡ Real-time power usage monitoring
- 🔍 mDNS support for easy device discovery
- 💾 Persistent configuration storage
- 🎨 Full RGB color and brightness control

## Hardware Requirements

- ESP32 development board (tested with ESP32-DOIT-DevKit-V1)
- WS2812B LED strip
- 5V power supply (rated for your LED strip length)

## Wiring

- Connect LED strip data input to GPIO 5
- Connect LED strip power to 5V power supply
- Connect LED strip ground to ESP32 ground and power supply ground

## Installation

### Prerequisites

1. Install [PlatformIO](https://platformio.org/install)
2. Libraries (will be installed automatically at build time):
   - WiFiManager (custom fork)
   - FastLED
   - ArtnetWifi
   - ArduinoJson

### Building and Flashing

1. Clone the repository:

2. Build and upload the firmware:
```bash
pio run --target upload
```

## Provisioning 
### Option 1: Configuration using configuration portal in access point mode

1. Power on the device for the first time
2. Connect to the WiFi network named "ArtNet_LED_Strip"
3. Open a web browser and navigate to 192.168.4.1
4. Configure your settings:
   - WiFi credentials
   - Number of LEDs
   - Art-Net universe
   - DMX start address
   - Device name

### Option 2: Device provisioning by SPIFFS upload

Use the provided `provision_device.sh` script to configure the device. This is particularly useful for provisioning multiple devices after flashing.

```bash
./provision_device.sh --num-leds 60 --universe 0 --address 1 --device-name "MyLEDs" --ssid "MyWiFi" --password "MyPassword"
```

For help with provisioning options:
```bash
./provision_device.sh --help
```
## Configuration Portal
The configuration portal can be accessed at `http://[device-name].local` at any time to change settings.

### WiFi

- Supports multiple WiFi networks with priority levels
- Automatically connects to the highest priority available network
- Falls back to configuration portal if no known networks are available

### Art-Net

- Universe: Configurable (default: 0)
- Start Address: Configurable (default: 1)
- Channels used: 4 (Brightness, Red, Green, Blue)

### Power Monitoring

The device calculates power usage based on:
- Red: 110mW per LED at full brightness
- Green: 79mW per LED at full brightness
- Blue: 103mW per LED at full brightness

Monitor real-time power usage through the web interface at `http://[device-name].local/monitor`

## OTA Updates

- Firmware updates can be performed over-the-air
- Upload the firmware using platformio:
```bash
pio run --target upload --upload-port [device-name].local 
```


## Temperature Measurements of 36 LEDs in case
- RGB (255,0,0); 4.2 W calculated, 57°C measured after 10 minutes, Room temperature 20°C
- RGB (0,255,0); 3.0 W calculated, 52°C measured after 10 minutes
- BGB (0,0,255); 3.9 W calculated, 54°C measured after 10 minutes
- RGB (255,0,255); 8.1 W calculated, 71°C measured after 10 minutes
