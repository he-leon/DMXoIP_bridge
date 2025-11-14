# DMXoIP Bridge – ESP32 Lighting Controller

Status: WIP but functional

This project turns an **ESP32** into a flexible lighting bridge that receives DMX-over-IP data and outputs it to LEDs or DMX devices. It supports multiple protocols, output types, and includes an onboard Wi-Fi setup page.

## 🚀 What It Does

The ESP32 acts as a DMX-over-IP receiver and lighting controller. It can receive:

* **Art-Net**
* **sACN / E1.31**
* **DMX_ESPNOW (wireless, low-latency)**

And it outputs to:

* **NeoPixel / WS2812 LEDs**
* **DMX512 (hardware serial)**
* **DMX_ESPNOW broadcast**

## ✨ Features

### Wi-Fi Setup with WiFiManager

* Automatic AP mode on first boot
* Configure:

  * LED count
  * DMX universe & start address
  * Output mode
  * Color mode
  * Device name
* Settings stored in SPIFFS

### Supported Input Protocols

| Protocol         | Description                               |
| ---------------- | ----------------------------------------- |
| **Art-Net**      | Widely used DMX-over-IP lighting protocol |
| **E1.31 (sACN)** | Efficient streaming ACN protocol          |
| **DMX_ESPNOW**      | Fast wireless peer-to-peer frame delivery |

### Output Options

| Output Mode        | Function                         |
| ------------------ | -------------------------------- |
| **NeoPixel**       | Drives WS2812/WS2812B LED strips |
| **DMX512**         | Outputs DMX via UART             |
| **DMX_ESPNOW Output** | Broadcasts DMX frames wirelessly |

## 🗂 Project Structure

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


## 🧩 How to Use

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

## 📦 Requirements

* **ESP32 board**
* Optional:

  * WS2812/NeoPixel LEDs
  * RS485 DMX transceiver module (for DMX output)
  * Additional ESP32 (for DMX_ESPNOW DMX networks)
