# SentryScope: Low-Power Perimeter Monitoring

An ESP32-based multi-sensor intrusion detection and smart camera system. SentryScope fuses three independent sensing modes — IR proximity, IR break-beam, and ultrasonic ranging — to validate intrusion events, then automatically orients a camera toward the detected zone, captures an image, and serves a live MJPEG stream over Wi-Fi.

Built for **BECE320E – Embedded C Programming**, School of Electronics Engineering, VIT Chennai (April 2026).

>  Full project report (design, literature survey, simulation, hardware results): [`docs/SentryScope_Project_Report.pdf`](docs/SentryScope_Project_Report.pdf)

---

## Why

Traditional CCTV-style systems record continuously, wasting power, storage, and bandwidth. Single-sensor triggers (PIR-only, IR-only) are also prone to false alarms from lighting changes or environmental noise.

SentryScope instead:
- **Fuses three sensors** so an alert only fires on corroborated detection, cutting false positives.
- **Only activates the camera on a verified event** (event-driven, not continuous), saving power and storage.
- **Auto-aims the camera** at the triggered zone using a servo before capturing.
- Runs the whole detect → aim → capture → reset cycle through a **Finite State Machine (FSM)** for predictable, non-overlapping timing (~500–700 ms end-to-end).

## Features

- Multi-sensor fusion: IR proximity + IR break-beam + HC-SR04 ultrasonic
- FSM-controlled operation: `READY → TARGET_SETTLE → CAPTURE_HOLD → RETURN_SETTLE → COOLDOWN`
- Servo-driven camera orientation (PWM, per-sensor target angle)
- ESP32-CAM triggered image capture, stored to microSD
- Live MJPEG stream over Wi-Fi with flash-on-trigger highlighting
- Debounced sensor reads and a cooldown window to prevent duplicate triggers
- Two-board architecture: a sensor/logic node (ESP32 DevKit) + a dedicated camera node (ESP32-CAM), communicating over a single GPIO trigger line

## System Architecture

```
                    ┌────────────────────────┐
   IR Proximity ───▶│                        │
   IR Break-Beam ──▶│      ESP32 DevKit      │──PWM──▶ Servo Motor (camera aim)
   Ultrasonic ─────▶│   (Sensing + FSM +     │
                    │     Decision Logic)     │──GPIO trigger──▶ ESP32-CAM
                    └────────────────────────┘                     │
                                                                     ├──▶ microSD (image storage)
                                                                     └──▶ Wi-Fi MJPEG livestream (browser)
```

**Flow:** sensors → ESP32 DevKit reads & debounces → FSM identifies trigger source → servo rotates to the mapped angle → DevKit pulses the trigger line → ESP32-CAM captures a frame (flashes an onboard LED) → servo returns home → cooldown → back to `READY`.

## Hardware

| Component | Spec / Notes | Approx. Cost (₹) |
|---|---|---|
| ESP32 DevKit V1 | Dual-core, controls sensors + FSM + servo | 350 |
| ESP32-CAM (AI Thinker) | OV2640, Wi-Fi livestream + capture | 600 |
| IR Proximity Sensor | Digital out, 3.3–5 V | 100 |
| IR Break-Beam Sensor (SEN0503) | TX/RX pair, active-low output | 150 |
| Ultrasonic Sensor (HC-SR04) | 2–400 cm range | 100 |
| Servo Motor (MG90S, 180°) | PWM camera positioning | 200 |
| MicroSD Card (16 GB) | Image storage | 250 |
| Power Supply (USB / 18650) | 5 V regulated | 700 |
| Misc. (wiring, capacitors, enclosure, FTDI) | — | ~660 |
| **Total** | | **~₹3000** |

Full BOM and component specs are in the [project report](docs/SentryScope_Project_Report.pdf) (§1.3.2, §4.1.3).

### Pinout — ESP32 DevKit (sensor/logic node)

| Signal | GPIO |
|---|---|
| IR Proximity | 33 |
| IR Break-Beam | 26 |
| Ultrasonic TRIG | 5 |
| Ultrasonic ECHO | 18 (via voltage divider, 5 V → 3.3 V) |
| Status LED | 25 |
| Servo (PWM) | 14 |
| Camera trigger out | 23 (open-drain) |

### Pinout — ESP32-CAM (AI Thinker)

| Signal | GPIO |
|---|---|
| Trigger in (from DevKit) | 13 |
| Flash LED | 4 |
| Camera data/clock pins | see [`firmware/esp32cam_streamer/camera_pins.h`](firmware/esp32cam_streamer/camera_pins.h) |

> ⚠️ Tie the DevKit's trigger-out GND to the ESP32-CAM's GND — the trigger line is a shared open-drain/interrupt signal between the two boards.

## Repository Structure

```
SentryScope-Perimeter-Monitoring/
├── README.md
├── docs/
│   └── SentryScope_Project_Report.pdf        # Full report: design, survey, results
├── firmware/
│   ├── devkit_controller/
│   │   └── devkit_controller.ino              # Sensors + FSM + servo + trigger (was sentinal_cam.ino)
│   └── esp32cam_streamer/
│       ├── esp32cam_streamer.ino               # Lightweight capture + MJPEG stream (was CameraWeb.ino)
│       └── camera_pins.h                       # AI Thinker pin map
├── reference/
│   └── esp32cam_webserver_example/             # Stock Espressif CameraWebServer example (kept for reference)
│       ├── app_httpd.cpp
│       ├── camera_index.h
│       ├── board_config.h
│       ├── camera_pins.h
│       ├── partitions.csv
│       └── ci.yml
└── images/
    ├── hardware_setup.jpg
    └── livestream_demo.jpg
```

> The `reference/esp32cam_webserver_example/` folder is Espressif's official `CameraWebServer` example (Apache License 2.0) — kept as a reference/fallback sketch, not required to build the project. The actual capture + streaming logic used in this project is the smaller custom sketch in `firmware/esp32cam_streamer/`.

### File renaming note
Arduino requires a sketch's `.ino` filename to match its parent folder name. When uploading, rename:
- `sentinal_cam.ino` → `firmware/devkit_controller/devkit_controller.ino`
- `CameraWeb.ino` → `firmware/esp32cam_streamer/esp32cam_streamer.ino`
- `REPORT_EMBC.pdf` → `docs/SentryScope_Project_Report.pdf`

## Getting Started

### Requirements
- Arduino IDE (or Arduino CLI) with the **ESP32 board package** installed
- Boards: 1× ESP32 DevKit, 1× ESP32-CAM (AI Thinker)
- FTDI/USB-TTL adapter to program the ESP32-CAM
- Libraries used: `WiFi.h`, `WebServer.h`, `esp_camera.h` (bundled with the ESP32 core)

### 1. Flash the ESP32-CAM
1. Open `firmware/esp32cam_streamer/esp32cam_streamer.ino` in Arduino IDE.
2. Set your Wi-Fi credentials:
   ```cpp
   const char* ssid = "YOUR_WIFI_SSID";
   const char* password = "YOUR_WIFI_PASSWORD";
   ```
3. Board settings: `AI Thinker ESP32-CAM`, Partition Scheme: **Huge APP (3MB No OTA/1MB SPIFFS)**.
4. Wire GPIO 0 to GND to enter flashing mode, upload, then remove the jumper and reset.
5. Open the Serial Monitor (115200 baud) to read the assigned IP address once connected.

### 2. Flash the ESP32 DevKit
1. Open `firmware/devkit_controller/devkit_controller.ino`.
2. Wire sensors and the servo per the pinout table above.
3. Select your DevKit board, upload.
4. Open the Serial Monitor (115200 baud) to watch sensor states and FSM transitions live.

### 3. Run it
1. Power both boards and connect the shared trigger line + common ground.
2. Visit `http://<esp32-cam-ip>/` in a browser on the same network to view the livestream.
3. Trigger any sensor — the servo aims, the DevKit pulses the trigger line, the CAM flashes and captures a frame, and the system resets after cooldown.

## Software Design — FSM States

| State | Action |
|---|---|
| `READY` | Poll all three sensors; on detection, map the triggered sensor to a servo angle |
| `TARGET_SETTLE` | Hold while servo moves to target angle |
| `CAPTURE_HOLD` | Pulse the camera trigger line, hold for capture |
| `RETURN_SETTLE` | Servo returns to home position |
| `COOLDOWN` | Ignore new triggers briefly to prevent duplicate captures, then back to `READY` |

## Results

- End-to-end response time: **~500–700 ms** (detection → servo aim → capture)
- Multi-sensor fusion measurably reduced false positives vs. single-sensor testing
- Stable MJPEG livestream over local Wi-Fi, accessible from any browser on the network

See §4.2–4.3 of the [project report](docs/SentryScope_Project_Report.pdf) for full test results and hardware photos.

## Limitations

- IR break-beam only covers a fixed line-of-sight path; ultrasonic range is short-distance only
- IR sensors are sensitive to ambient lighting; ultrasonic readings vary with surface/material
- No cloud alerts yet — images are stored locally only (no push notifications)
- ESP32-CAM's limited compute rules out on-device AI/object detection for now

## Future Scope

- IoT/cloud integration for remote alerts (MQTT, Firebase, Blynk)
- On-device or edge AI-based object/person classification
- Long-range links (LoRa/GSM) for connectivity-limited deployments
- Closed-loop/auto-calibrating servo positioning
- Battery/solar power optimization for fully off-grid operation

## Authors

- Gopikasree R (23BEC1013)
- Kirthana S (23BEC1412)
- Harshitha Senthil Kumar (23BEC1428)
- Joshitha G (23BEC1478)

Guided by **Dr. A. Sivasubramanian**, School of Electronics Engineering, VIT Chennai.


## Acknowledgements

- Espressif Systems — ESP32 Arduino core and `CameraWebServer` example (used as reference for camera server setup)
- [Random Nerd Tutorials — ESP32-CAM video streaming](https://randomnerdtutorials.com/esp32-cam-video-streaming-face-recognition-arduino-ide/)
