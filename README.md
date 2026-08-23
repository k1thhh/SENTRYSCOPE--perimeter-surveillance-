# SentryScope: Low-Power Perimeter Monitoring

<p align="center">

**Embedded Security System | Multi-Sensor Fusion | ESP32 | ESP32-CAM | FSM | IoT**

</p>

<p align="center">
  <img src="https://img.shields.io/badge/Domain-Embedded%20Systems-blue" alt="Embedded Systems">
  <img src="https://img.shields.io/badge/Platform-ESP32-orange" alt="ESP32">
  <img src="https://img.shields.io/badge/Camera-ESP32--CAM-green" alt="ESP32-CAM">
  <img src="https://img.shields.io/badge/Language-Embedded%20C-red" alt="Embedded C">
  <img src="https://img.shields.io/badge/Detection-Multi--Sensor%20Fusion-purple" alt="Multi-Sensor Fusion">
</p>

---

## 📌 Overview

**SentryScope** is a low-power, event-driven perimeter monitoring and intrusion detection system built around an **ESP32 DevKit and ESP32-CAM**.

Instead of continuously recording video like a conventional CCTV system, SentryScope combines **three independent sensing modes — IR proximity, IR break-beam, and ultrasonic ranging — to validate intrusion events before activating the camera**.

When an intrusion is detected, the ESP32 DevKit identifies the triggering sensor, maps it to a corresponding camera position, rotates a servo toward the detected zone, and triggers the ESP32-CAM to capture an image. The camera simultaneously provides a **live MJPEG video stream over Wi-Fi**, while captured images are stored on a microSD card.

The complete detection-to-capture process is controlled using a **Finite State Machine (FSM)**, providing predictable timing and preventing overlapping events.

The system achieves an approximately **500–700 ms end-to-end response time** from detection to camera capture.

Built as part of **BECE320E – Embedded C Programming, School of Electronics Engineering, VIT Chennai (April 2026)**.

---

# 🔐 Why SentryScope?

Traditional CCTV-based perimeter monitoring systems continuously record video, even when no activity is occurring.

This results in:

* Unnecessary power consumption
* Continuous storage requirements
* Higher bandwidth usage
* Large amounts of irrelevant footage

Similarly, systems relying on a single sensor can produce false alarms due to:

* Ambient lighting changes
* Environmental noise
* Sensor limitations
* Temporary disturbances

SentryScope addresses these limitations through an **event-driven multi-sensor architecture**.

```text
             Traditional CCTV
                    │
                    ▼
          Continuous Recording
                    │
        ┌───────────┼───────────┐
        ▼           ▼           ▼
      Power       Storage     Bandwidth
      Usage        Usage        Usage
```

SentryScope instead follows:

```text
Multiple Sensors
       │
       ▼
Sensor Fusion
       │
       ▼
Verified Intrusion
       │
       ▼
Camera Activation
       │
       ▼
Targeted Capture
```

This allows the camera to remain inactive until a meaningful event is detected.

---

# 🎯 Project Objectives

The primary objectives of SentryScope are:

1. Detect perimeter intrusion using multiple independent sensors.
2. Reduce false positives through sensor fusion.
3. Implement event-driven rather than continuous camera activation.
4. Automatically orient the camera toward the detected zone.
5. Capture images only after a validated detection event.
6. Store captured images locally on a microSD card.
7. Provide a live MJPEG camera stream over Wi-Fi.
8. Implement predictable event handling using a Finite State Machine.
9. Prevent duplicate triggers using debouncing and cooldown logic.
10. Build a low-cost embedded perimeter monitoring prototype.

---

# 🏗️ System Architecture

SentryScope uses a **two-board architecture**.

The first board is an **ESP32 DevKit**, responsible for sensing, decision-making, FSM control, and servo positioning.

The second board is an **ESP32-CAM**, responsible for image capture, flash control, microSD storage, and Wi-Fi streaming.

```text
                    ┌────────────────────────┐
   IR Proximity ───▶│                        │
   IR Break-Beam ──▶│      ESP32 DevKit      │
   Ultrasonic ─────▶│   Sensing + FSM +      │
                    │   Decision Logic       │
                    └──────────┬─────────────┘
                               │
                       PWM     │     GPIO Trigger
                       │       │
                       ▼       └─────────────────────┐
                ┌────────────┐                       ▼
                │   Servo    │                ┌──────────────┐
                │   Motor    │                │  ESP32-CAM   │
                └────────────┘                │              │
                                              │   OV2640     │
                                              └──────┬───────┘
                                                     │
                                    ┌────────────────┼──────────────┐
                                    ▼                ▼              ▼
                                  microSD          Wi-Fi        Flash LED
                                    │                │
                                    ▼                ▼
                               Image Storage    MJPEG Stream
                                                     │
                                                     ▼
                                                  Browser
```

---

# 🔄 System Operating Flow

The complete operating sequence is:

```text
             Sensors
                │
                ▼
        Sensor Read + Debounce
                │
                ▼
        Intrusion Detection
                │
                ▼
        Identify Trigger Zone
                │
                ▼
        Map Zone → Servo Angle
                │
                ▼
        Servo Moves to Target
                │
                ▼
        GPIO Camera Trigger
                │
                ▼
        ESP32-CAM Captures
                │
        ┌───────┴────────┐
        ▼                ▼
    Flash LED         Save Image
                         │
                         ▼
                  Servo Returns Home
                         │
                         ▼
                      Cooldown
                         │
                         ▼
                       READY
```

The complete detection-to-reset cycle takes approximately **500–700 ms**.

---

# 🧩 Multi-Sensor Detection

SentryScope uses three independent sensing mechanisms.

## IR Proximity Sensor

The IR proximity sensor detects nearby objects using infrared reflection.

It provides a simple digital trigger indicating that an object has entered the monitored region.

---

## IR Break-Beam Sensor

The IR break-beam sensor uses a transmitter/receiver pair.

When an object interrupts the infrared beam, the sensor produces an active-low detection signal.

This provides a fixed line-of-sight intrusion boundary.

---

## HC-SR04 Ultrasonic Sensor

The HC-SR04 measures distance using ultrasonic pulses.

It provides a detection mechanism based on the distance between the sensor and an object.

The sensor has an approximate operating range of **2–400 cm**.

---

# 🧠 Sensor Fusion

Rather than relying on a single sensor, SentryScope combines:

```text
IR Proximity
      +
IR Break-Beam
      +
Ultrasonic
      │
      ▼
Detection Logic
      │
      ▼
Validated Event
```

This provides greater robustness against individual sensor errors.

The project reports that multi-sensor fusion **measurably reduced false positives compared with single-sensor testing**.

---

# ⚙️ Finite State Machine

The detection and camera-control sequence is implemented using a **Finite State Machine (FSM)**.

The system contains five primary states:

```text
                 ┌─────────────┐
                 │    READY    │
                 └──────┬──────┘
                        │
                  Detection
                        ▼
              ┌─────────────────┐
              │ TARGET_SETTLE   │
              └────────┬────────┘
                       │
                       ▼
              ┌─────────────────┐
              │  CAPTURE_HOLD   │
              └────────┬────────┘
                       │
                       ▼
              ┌─────────────────┐
              │ RETURN_SETTLE   │
              └────────┬────────┘
                       │
                       ▼
              ┌─────────────────┐
              │    COOLDOWN     │
              └────────┬────────┘
                       │
                       ▼
                   READY
```

---

## 1️⃣ READY

The system continuously polls the three sensors.

When an intrusion is detected:

* The triggering sensor is identified.
* The corresponding camera zone is selected.
* A target servo angle is assigned.
* The FSM transitions to `TARGET_SETTLE`.

---

## 2️⃣ TARGET_SETTLE

The servo rotates toward the detected zone.

The system waits for the servo to settle before triggering the camera.

This prevents the camera from capturing while the mechanism is still moving.

---

## 3️⃣ CAPTURE_HOLD

The ESP32 DevKit pulses the camera trigger line.

The ESP32-CAM:

* Receives the trigger
* Activates the flash LED
* Captures an image
* Stores the image on microSD

---

## 4️⃣ RETURN_SETTLE

After the capture event, the servo returns to its home position.

The system waits for the mechanism to settle before accepting another event.

---

## 5️⃣ COOLDOWN

A short cooldown period prevents duplicate triggers caused by the same physical event.

After cooldown expires, the system transitions back to:

```text
READY
```

---

# 📐 Servo-Based Camera Orientation

The camera is mounted on a servo motor controlled using PWM.

Each sensing region is mapped to a corresponding servo angle.

```text
             Camera
                ▲
                │
          ┌─────┴─────┐
          │   Servo   │
          └─────┬─────┘
                │
       ┌────────┼────────┐
       ▼        ▼        ▼
     Zone 1   Zone 2   Zone 3
       ▲        ▲        ▲
       │        │        │
      IR      Beam    Ultrasonic
```

When a sensor is triggered, the camera is automatically oriented toward the associated detection zone.

This provides **targeted surveillance rather than fixed-direction monitoring**.

---

# 📷 ESP32-CAM Subsystem

The ESP32-CAM acts as the dedicated imaging node.

Its responsibilities include:

* Image acquisition
* Camera control
* Flash activation
* microSD storage
* Wi-Fi connectivity
* MJPEG streaming
* External trigger handling

The project uses the **AI Thinker ESP32-CAM with an OV2640 camera**.

---

# 🌐 Live MJPEG Streaming

The ESP32-CAM provides a live **MJPEG video stream** over Wi-Fi.

The stream can be accessed from a browser connected to the same network.

```text
ESP32-CAM
    │
    ▼
Camera Frames
    │
    ▼
MJPEG Encoding
    │
    ▼
Wi-Fi
    │
    ▼
Browser
```

This provides real-time visual monitoring while the event-driven capture mechanism handles intrusion events.

---

# 💾 Image Storage

Captured images are stored locally on a **16 GB microSD card**.

This allows detected events to be retained for later inspection without requiring continuous cloud connectivity.

The architecture therefore supports:

```text
Intrusion Event
      │
      ▼
Camera Capture
      │
      ▼
microSD
      │
      ▼
Stored Evidence
```

---

# 🔌 Two-Board Communication

The DevKit and ESP32-CAM communicate through a **single GPIO trigger line**.

The DevKit generates the trigger after validating an intrusion.

```text
ESP32 DevKit
     │
     │ GPIO Trigger
     ▼
ESP32-CAM
     │
     ▼
Image Capture
```

The trigger line uses an open-drain configuration.

A common ground between the two boards is required for reliable communication.

---

# 🛠️ Hardware

| Component                | Function                                                 |
| ------------------------ | -------------------------------------------------------- |
| **ESP32 DevKit V1**      | Sensor processing, FSM, decision logic and servo control |
| **ESP32-CAM**            | Camera capture, Wi-Fi streaming and image storage        |
| **IR Proximity Sensor**  | Proximity-based intrusion detection                      |
| **IR Break-Beam Sensor** | Beam-interruption detection                              |
| **HC-SR04**              | Ultrasonic distance measurement                          |
| **MG90S Servo**          | Camera positioning                                       |
| **OV2640**               | Image acquisition                                        |
| **MicroSD Card**         | Local image storage                                      |
| **FTDI / USB-TTL**       | ESP32-CAM programming                                    |
| **Power Supply**         | System power                                             |

---

# 💰 Hardware Cost

The approximate project cost is:

| Component           | Approx. Cost |
| ------------------- | -----------: |
| ESP32 DevKit V1     |         ₹350 |
| ESP32-CAM           |         ₹600 |
| IR Proximity Sensor |         ₹100 |
| IR Break-Beam       |         ₹150 |
| HC-SR04             |         ₹100 |
| MG90S Servo         |         ₹200 |
| 16 GB microSD       |         ₹250 |
| Power Supply        |         ₹700 |
| Miscellaneous       |        ~₹660 |
| **Total**           |   **~₹3000** |

This makes SentryScope a relatively low-cost prototype for event-driven perimeter monitoring.

---

# 📍 Pin Configuration

## ESP32 DevKit

| Signal          | GPIO |
| --------------- | ---: |
| IR Proximity    |   33 |
| IR Break-Beam   |   26 |
| Ultrasonic TRIG |    5 |
| Ultrasonic ECHO |   18 |
| Status LED      |   25 |
| Servo PWM       |   14 |
| Camera Trigger  |   23 |

The HC-SR04 echo signal uses a voltage divider to convert the 5 V output to a safe 3.3 V level.

---

## ESP32-CAM

| Signal           |                          GPIO |
| ---------------- | ----------------------------: |
| Trigger Input    |                            13 |
| Flash LED        |                             4 |
| Camera Interface | AI Thinker camera pin mapping |

---

# 💻 Software Design

The software is divided between two firmware programs.

### ESP32 DevKit

Responsible for:

* Sensor acquisition
* Sensor debouncing
* Intrusion detection
* Sensor fusion
* FSM execution
* Servo control
* Camera triggering
* Cooldown management

### ESP32-CAM

Responsible for:

* Camera initialization
* Image capture
* Flash control
* microSD storage
* Wi-Fi connection
* MJPEG streaming
* Trigger handling

---

# 🧰 Tools & Technologies

## Embedded C / Arduino

The firmware is developed using the Arduino framework for ESP32 and written in embedded C/C++.

---

## ESP32

The ESP32 DevKit provides:

* GPIO
* PWM
* Sensor interfacing
* Embedded processing
* FSM execution

---

## ESP32-CAM

The ESP32-CAM provides:

* OV2640 camera interface
* Wi-Fi
* microSD interface
* Image capture
* Streaming capability

---

## Wi-Fi

Wi-Fi is used to provide local network access to the camera's MJPEG stream.

---

## PWM

PWM is used to control the MG90S servo and orient the camera toward the detected region.

---

# 📁 Project Structure

```text
SentryScope-Perimeter-Monitoring/
│
├── README.md
│
├── docs/
│   └── SentryScope_Project_Report.pdf
│
├── firmware/
│   ├── devkit_controller/
│   │   └── devkit_controller.ino
│   │
│   └── esp32cam_streamer/
│       ├── esp32cam_streamer.ino
│       └── camera_pins.h
│
├── reference/
│   └── esp32cam_webserver_example/
│       ├── app_httpd.cpp
│       ├── camera_index.h
│       ├── board_config.h
│       ├── camera_pins.h
│       ├── partitions.csv
│       └── ci.yml
│
└── images/
    ├── hardware_setup.jpg
    └── livestream_demo.jpg
```

The `reference/` directory contains Espressif's CameraWebServer example for reference and fallback purposes, while the actual project uses the smaller custom camera-streaming implementation.

---

# 🧰 Installation & Environment

## Requirements

The project requires:

* Arduino IDE or Arduino CLI
* ESP32 board package
* ESP32 DevKit V1
* AI Thinker ESP32-CAM
* FTDI / USB-TTL adapter
* Sensors and servo
* microSD card

The required ESP32 libraries include:

```text
WiFi.h
WebServer.h
esp_camera.h
```

These are provided through the ESP32 Arduino core.

---

# ⚙️ ESP32-CAM Setup

1. Open:

```text
firmware/esp32cam_streamer/esp32cam_streamer.ino
```

2. Configure the Wi-Fi credentials:

```cpp
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
```

3. Select:

```text
Board: AI Thinker ESP32-CAM
Partition: Huge APP
```

4. Connect GPIO 0 to GND to enter flashing mode.

5. Upload the firmware.

6. Remove the GPIO 0 jumper.

7. Reset the board.

8. Open Serial Monitor at:

```text
115200 baud
```

9. Note the IP address assigned to the ESP32-CAM.

---

# ⚙️ ESP32 DevKit Setup

1. Open:

```text
firmware/devkit_controller/devkit_controller.ino
```

2. Connect the sensors according to the pinout.

3. Connect the servo to GPIO 14.

4. Connect the camera trigger output to GPIO 23.

5. Upload the firmware.

6. Open Serial Monitor at:

```text
115200 baud
```

The Serial Monitor can be used to observe sensor states and FSM transitions.

---

# ▶️ Running the System

Once both boards have been programmed:

```text
Power ON
   │
   ▼
ESP32 DevKit
   │
   ├── Sensors Active
   │
   └── FSM = READY
   │
   ▼
ESP32-CAM
   │
   └── Wi-Fi + MJPEG Server
```

Open:

```text
http://<ESP32-CAM-IP>/
```

from a browser connected to the same Wi-Fi network.

When a sensor detects an intrusion:

```text
Detection
    ↓
Servo Aim
    ↓
Camera Trigger
    ↓
Flash
    ↓
Image Capture
    ↓
microSD Storage
    ↓
Servo Return
    ↓
Cooldown
    ↓
READY
```

---

# 🔍 Verification Strategy

SentryScope can be evaluated at several levels.

### Sensor Level

Verify that each sensor correctly detects its intended intrusion condition.

### Fusion Level

Compare multi-sensor operation against individual sensor operation to evaluate false-positive reduction.

### FSM Level

Observe state transitions during a complete detection cycle.

### Servo Level

Verify that each trigger source moves the camera toward its mapped zone.

### Camera Level

Verify that a trigger produces a flash and image capture.

### Storage Level

Verify that captured frames are successfully stored on the microSD card.

### Network Level

Verify that the MJPEG stream remains accessible over the local Wi-Fi network.

### Timing Level

Measure the complete detection-to-capture response.

---

# 📊 Project Results

The implemented system achieves:

| Parameter                  | Result                                      |
| -------------------------- | ------------------------------------------- |
| Architecture               | Two-board ESP32 system                      |
| Sensor Types               | 3                                           |
| Camera                     | ESP32-CAM + OV2640                          |
| Image Storage              | microSD                                     |
| Streaming                  | Wi-Fi MJPEG                                 |
| Camera Positioning         | Servo-controlled                            |
| Control Architecture       | FSM                                         |
| Response Time              | **~500–700 ms**                             |
| Detection Strategy         | Multi-sensor fusion                         |
| False Positive Performance | Reduced compared with single-sensor testing |
| Approx. Hardware Cost      | **~₹3000**                                  |

---

# ⚡ Low-Power Event-Driven Architecture

One of the key design principles is to avoid unnecessary camera operation.

Instead of:

```text
Camera
  │
  ▼
Continuous Recording
  │
  ▼
Continuous Storage
```

SentryScope uses:

```text
Sensors
  │
  ▼
Event Detection
  │
  ▼
Camera Activation
  │
  ▼
Capture
  │
  ▼
Return to Monitoring
```

This reduces unnecessary:

* Camera activity
* Storage usage
* Processing
* Bandwidth consumption

The design therefore focuses on **event-driven surveillance rather than continuous recording**.

---

# 🧠 Key Concepts Demonstrated

This project provides practical exposure to:

* Embedded C programming
* ESP32 development
* ESP32-CAM
* Sensor interfacing
* Multi-sensor fusion
* Intrusion detection
* Event-driven architecture
* Finite State Machines
* GPIO interrupts/triggers
* PWM servo control
* Camera control
* microSD storage
* Wi-Fi communication
* MJPEG streaming
* Sensor debouncing
* Cooldown logic
* Two-board embedded architectures
* Real-time embedded systems
* Low-power system design

---

# 💡 What I Learned

SentryScope demonstrates how multiple relatively simple sensors can be combined to create a more robust embedded monitoring system.

The overall design can be summarized as:

```text
       Sensing
          ↓
   Sensor Fusion
          ↓
     Decision Logic
          ↓
         FSM
          ↓
    Camera Position
          ↓
       Capture
          ↓
      Data Storage
          ↓
     Wi-Fi Streaming
```

The project also demonstrates an important embedded-systems principle:

> **Do computation and hardware activation only when an event requires it.**

Rather than continuously processing camera data, the system uses inexpensive sensors to identify when visual information is actually needed.

---

# 🏠 Applications

The architecture can be adapted for:

### Perimeter Security

* Residential boundaries
* Restricted areas
* Small facilities
* Storage areas

### Smart Surveillance

* Event-triggered CCTV
* Low-power monitoring
* Remote visual inspection

### Industrial Monitoring

* Equipment-area monitoring
* Restricted-zone detection
* Automated event capture

### IoT Security

* Wi-Fi-enabled monitoring
* Remote event logging
* Cloud-connected surveillance

---

# ⚠️ Limitations

The current implementation has several limitations.

### Sensor Coverage

The IR break-beam only covers a fixed line-of-sight path.

### Ultrasonic Range

The HC-SR04 provides short-distance detection and can behave differently depending on object surface and material.

### Environmental Sensitivity

IR sensors can be affected by ambient lighting conditions.

### Local Storage

Images are stored locally rather than being automatically uploaded to a cloud service.

### Limited Edge Processing

The ESP32-CAM's computational resources currently limit the use of sophisticated on-device AI or object-detection models.

---

# 🚀 Future Scope

Potential extensions include:

* MQTT-based IoT integration
* Firebase connectivity
* Blynk-based remote monitoring
* Cloud image storage
* Push notifications
* Person/object classification
* Edge AI-based intrusion verification
* LoRa-based long-range communication
* GSM-based remote alerts
* Automatic servo calibration
* Battery optimization
* Solar-powered operation
* Fully off-grid deployment

A future AI-enabled version could introduce another validation stage:

```text
Sensor Detection
       ↓
Camera Capture
       ↓
Edge AI
       ↓
Person/Object Classification
       ↓
Verified Security Event
       ↓
Remote Alert
```

---

# ⭐ Project Highlights

* 🔹 **Three-sensor intrusion detection**
* 🔹 IR proximity + IR break-beam + ultrasonic sensing
* 🔹 **Multi-sensor fusion** for improved detection reliability
* 🔹 Event-driven camera activation
* 🔹 **Finite State Machine-based control**
* 🔹 Automatic servo-based camera orientation
* 🔹 ESP32 + ESP32-CAM two-board architecture
* 🔹 OV2640 image capture
* 🔹 microSD image storage
* 🔹 Live **MJPEG Wi-Fi streaming**
* 🔹 Flash-on-trigger functionality
* 🔹 Sensor debouncing
* 🔹 Cooldown-based duplicate-event prevention
* 🔹 Approximately **500–700 ms response time**
* 🔹 Low-cost implementation of approximately **₹3000**
* 🔹 Designed and implemented using Embedded C

---

# 📚 References

* Espressif Systems — ESP32 Arduino Core
* Espressif Systems — CameraWebServer example
* Random Nerd Tutorials — ESP32-CAM video streaming reference
* SentryScope Project Report
* BECE320E — Embedded C Programming
* School of Electronics Engineering, VIT Chennai

---

# 👥 Authors

**Gopikasree R**
**Kirthana S**
**Harshitha Senthil Kumar**
**Joshitha G**

Guided by **Dr. A. Sivasubramanian**, School of Electronics Engineering, VIT Chennai.

---

# 📌 Keywords

`SentryScope` `ESP32` `ESP32-CAM` `Embedded C` `Embedded Systems` `Perimeter Security` `Intrusion Detection` `Sensor Fusion` `IR Sensor` `Break Beam` `HC-SR04` `Ultrasonic` `Servo` `PWM` `Finite State Machine` `FSM` `Wi-Fi` `MJPEG` `Camera` `microSD` `IoT` `Event-Driven System` `Low-Power Monitoring` `Smart Surveillance`

---

<p align="center">

**Sense → Validate → Aim → Capture → Store → Stream**

</p>
