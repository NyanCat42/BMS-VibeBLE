# 📦 BMS-VibeBLE

**BMS-VibeBLE** is a *_vibe coded_* Arduino library that connects to JBD/Overkill Solar-compatible BMSes via BLE using [NimBLE-Arduino](https://github.com/h2zero/NimBLE-Arduino) on ESP32 devices.

It reads and parses essential battery data including voltage, current, SOC, NTC temperatures, capacity, and more.

---

## ✅ Features

- Connects to BMS over BLE using MAC address
- Reads and parses:
  - Voltage, current
  - Remaining and full capacity
  - Cycle count
  - SoC (State of Charge)
  - Production date, software verion
  - Temperatures from all NTCs
  - Number of cells and NTCs
  - Balance, MOSFET and protection status
- Lightweight, no dynamic allocation of BLE services
- Designed for ESP32 using NimBLE (low RAM usage)

---

## 📥 Installation (PlatformIO)

You can add the library directly via GitHub by editing your `platformio.ini`:

```ini
lib_deps =
  https://github.com/NyanCat42/BMS-VibeBLE.git
