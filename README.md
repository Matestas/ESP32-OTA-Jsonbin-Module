# ESP32 OTA Jsonbin Module

This project enables Over-The-Air (OTA) firmware updates for ESP32 using metadata stored on [Jsonbin.io](https://jsonbin.io/). It is built using the **ESP-IDF** framework and performs OTA updates by fetching firmware version info and binary URLs from a JSON bin.

---

## Features

- OTA updates for ESP32 using ESP-IDF  
- Fetches firmware metadata from Jsonbin.io  
- Downloads and flashes new firmware if available  
- Uses secure HTTPS connections  

---

## Requirements

- **ESP32 development board**  
- [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/) (Espressif IoT Development Framework) installed and configured  
- A Jsonbin.io account and a JSON bin containing firmware metadata  
- Firmware binaries hosted on a publicly accessible HTTPS server  
- WiFi Connection (and previous setup)

## Setup Instructions

### 1. Prepare Firmware Metadata JSON

Create a JSON bin on [Jsonbin.io](https://jsonbin.io/) with the structure:

```json
{
  "version": 1.0,  # use only one dot
  "firmware_bin": "https://example.com/firmware.bin"
}
````

* `version`: semantic version string of the firmware
* `firmware_bin`: direct HTTPS URL to the firmware binary

### 2. Configure the Project

* Clone this repository and open it with ESP-IDF:

```bash
git clone https://github.com/Matestas/ESP32-OTA-Jsonbin-Module.git
cd ESP32-OTA-Jsonbin-Module
idf.py set-target esp32
```

### 3. Build and Flash Initial Firmware

* Build the project:

```bash
idf.py build
```

* Flash it to your device:

```bash
idf.py -p /dev/ttyUSB0 flash
```

(Change `/dev/ttyUSB0` to your serial port)

### 4. Deploy Firmware Binaries

* Upload compiled firmware `.bin` files to your hosting server (e.g., GitHub Releases or cloud storage)
* Update the JSON bin with the new `version` and `bin_url` each time you release a new firmware

### 5. OTA Update Workflow

* On startup, the ESP32 fetches JSON metadata from Jsonbin.io
* If a newer firmware version is detected, it downloads and flashes the update
* The device restarts with the updated firmware

---
Here’s a polished rewording of that section for clarity and flow:

---

## HTTPS Compatibility Notice

This OTA module currently supports HTTPS connections **only** with the following services:

* [Jsonbin.io](https://jsonbin.io/) for fetching firmware metadata JSON
* GitHub (e.g., GitHub Releases) for hosting firmware binary files

Support for other HTTPS providers or custom servers is **not guaranteed** because the CA certificates are hardcoded in `.pem` files and cannot be configured dynamically at this time. 

---

Want me to add this into your README?

## Notes on `main.c`

The included `main.c` file is just a simple test/demo application to show how the OTA module can be used. It is **not required** for the OTA module itself to function.

To run `main.c`, you will need the `wifi_connect` module I developed to manage WiFi connections. However, this module is **not available yet**, so `main.c` currently serves only as an example and won’t work out of the box.

---

## License

MIT License

```

This project is licensed under the MIT License.
```
