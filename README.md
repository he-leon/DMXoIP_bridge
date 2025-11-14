# DMXoIP Bridge – ESP32 Lighting Controller

Status: WIP but functional

This project turns an **ESP32** into a flexible lighting bridge that receives DMX-over-IP data and outputs it to LEDs or DMX devices. It supports multiple protocols, output types, and includes an onboard Wi-Fi setup page.

## What It Does

The ESP32 acts as a DMX-over-IP receiver and lighting controller. It can receive:

* **Art-Net**
* **sACN / E1.31**
* **DMX_ESPNOW (wireless, low-latency)**

And it outputs to:

* **NeoPixel / WS2812 LEDs**
* **DMX512 (hardware serial)**
* **DMX_ESPNOW broadcast**

## Features

### Wi-Fi Setup with WiFiManager

* Automatic AP mode on first boot
* Web-based configuration portal
* Configure:
  * Input protocol (Art-Net, sACN/E1.31, DMX_ESPNOW)
  * DMX universe
  * Device name
  * Output mode (NeoPixel, DMX512, DMX_ESPNOW)
  * NeoPixel: LED count and start address
  * NeoPixel: Color mode (individual or single combined)
* Settings stored in SPIFFS
* Over-the-air (OTA) firmware updates

### Supported Input Protocols

| Protocol         | Description                               |
| ---------------- | ----------------------------------------- |
| **Art-Net**      | Widely used DMX-over-IP lighting protocol |
| **E1.31 (sACN)** | Efficient streaming ACN protocol          |
| **DMX_ESPNOW**   | Fast wireless peer-to-peer frame delivery |

### Output Options

| Output Mode        | Function                         |
| ------------------ | -------------------------------- |
| **NeoPixel**       | Drives WS2812/WS2812B LED strips |
| **DMX512**         | Outputs DMX via UART             |
| **DMX_ESPNOW**     | Broadcasts DMX frames wirelessly |

## Project Structure

### Core Files

* **main.ino** — Main program entry, initializes all systems
* **ConfigParameters.*** — Stores and loads all device settings
* **NetworkConfig.*** — Web config pages, menus, OTA, JSON export
* **DMXoIPHandler.*** — Receives Art-Net, E1.31, and DMX_ESPNOW DMX
* **NeoPixelDMXFrameHandler.*** — Converts DMX → NeoPixel output
* **HardwareSerialDMXOutput.*** — Sends DMX512 frames over UART
* **ESPDMXNowFrameHandler.*** — Sends DMX over DMX_ESPNOW
* **LEDConfig.*** — LED setup, brightness, testing effects
* **Sensors/*** — Optional sensor integrations
* **StatusLED.*** — Diagnostic/status LED handling
* **interfaces/*** — Clean input/output interface classes

## How to Use

1. Flash the firmware onto an **ESP32**.
2. On first boot, connect to the ESP32 Wi-Fi AP.
3. Open the configuration portal.
4. Set:
   * Wi-Fi credentials
   * Output mode
   * LED count
   * Universe & start address
5. Save + reboot.
6. Send Art-Net or sACN from your lighting software to the device.

## Requirements

* **ESP32 board**
* Optional:
  * WS2812/NeoPixel LEDs
  * RS485 DMX transceiver module (for DMX512 output)
  * Additional ESP32 (for DMX_ESPNOW networks)

## Used Libraries

* **[WiFiManager](https://github.com/he-leon/WiFiManager)** - Web-based configuration portal for WiFi credentials
* **[ArtnetWifi](https://github.com/yakamoz423/ArtnetWifi)** - Art-Net protocol implementation
* **[ArduinoJson](https://arduinojson.org/)** - JSON parsing and generation
* **[DallasTemperature](https://github.com/milesburton/Arduino-Temperature-Control-Library)** - DS18B20 temperature sensor support
* **[OneWire](https://github.com/PaulStoffregen/OneWire)** - OneWire protocol for DS18B20 sensors
* **[NeoPixelBus](https://github.com/Makuna/NeoPixelBus)** - WS2812/NeoPixel LED control
* **[ESPAsyncE131](https://github.com/forkineye/ESPAsyncE131)** - sACN/E1.31 protocol implementation
* **[esp_dmxnow](https://github.com/he-leon/esp_dmxnow)** - Custom ESP-NOW DMX protocol implementation

## Installation

1. Install PlatformIO
2. Clone this repository
3. Run `pio run -t upload` to build and upload firmware
4. Connect to the ESP32 Wi-Fi network on first boot
5. Configure via web interface

## Configuration

The device can be configured through the web interface:

* **Wi-Fi Settings**: Connect to your network
* **Protocol Selection**: Choose input protocol (Art-Net, sACN/E1.31, DMX_ESPNOW)
* **Output Mode**: Select output type (NeoPixel, DMX512, DMX_ESPNOW)
* **LED Configuration**: Set number of LEDs and addressing mode
* **DMX Settings**: Configure universe and start address

## Protocols

### Art-Net

Standard DMX-over-IP protocol, widely supported by lighting software.

### sACN / E1.31

ETC's streaming ACN protocol, efficient for multicast DMX distribution.

### DMX_ESPNOW

Custom wireless DMX protocol using ESP-NOW for latency.

## Outputs

### NeoPixel / WS2812

Direct LED strip control with individual pixel addressing.

### DMX512

Standard professional lighting protocol via RS485 transceiver.

### DMX_ESPNOW

Wireless DMX broadcasting using ESP-NOW for peer-to-peer networks.
