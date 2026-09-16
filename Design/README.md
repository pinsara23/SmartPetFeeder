# 🐾 Smart PAW – Automatic Pet Feeder Mechanical Design

Smart PAW is an automated pet feeding system designed to dispense a predefined amount of food while controlling access to the feeding area.

This repository contains the mechanical/CAD design of the Smart PAW pet feeder.

The enclosure and internal mechanisms were designed to support the electronic components of the system, including:

- ESP32 microcontroller
- Load cells with HX711 modules
- Servo motors
- RFID reader
- PCA9685 servo controller
- 20×4 LCD display
- 4×4 keypad

The complete system combines embedded electronics, sensors, servo control, weight measurement, RFID access control, and mechanical design.

---

## 📸 Smart PAW CAD Design

![Smart PAW CAD Design](images/smart-paw-design.jpg)

The design contains three main sections:

1. Food Storage Container
2. Food Measuring / Dispensing Chamber
3. Pet Feeding Area

---

## 🏗️ Mechanical Design Overview

### 1. Food Storage Container

The upper section of the feeder acts as the main food storage hopper.

Food is stored here until the feeding process begins.

The bottom of the container guides the food toward the first servo-controlled gate.

        Food Storage
             │
             ▼
      Servo Controlled Gate
             │
             ▼
      Measuring Chamber

The sloped walls help guide food toward the dispensing opening using gravity.

---

### 2. First Servo Gate – Food Dispensing

A servo motor controls the opening between the food storage container and the measuring chamber.

When food is requested, the ESP32 gradually opens the servo gate.

Food falls into the measuring chamber while the system continuously monitors its weight.

The software uses different servo movements to improve dispensing accuracy and reduce food jams.

Features include:

- Small initial servo opening
- Gradual food dispensing
- Anti-jam servo movement
- Automatic stopping near the target weight

---

## ⚖️ Food Measuring Chamber

The middle section acts as the food measurement chamber.

Load cells installed underneath the chamber measure the amount of food that has been dispensed.

The load cells are connected to HX711 load-cell amplifier modules.

Food Hopper
     │
     ▼
Servo Gate
     │
     ▼
┌──────────────────┐
│ Measuring Chamber│
│                  │
│   Food + Weight  │
└──────────────────┘
        │
    Load Cells
        │
      HX711
        │
      ESP32

The ESP32 compares the measured weight with the target weight entered by the user.

For example:

Target Weight = 40 g

Current Weight = 10 g
→ Continue dispensing

Current Weight = 30 g
→ Reduce food flow

Current Weight = 38 g
→ Stop dispensing

A small tolerance is used so that the system does not continuously attempt to reach an exact measurement.

---

## 🔄 Anti-Jam Mechanism

Dry pet food can occasionally become stuck near the dispensing opening.

To reduce this problem, the feeder uses a software-controlled anti-jam mechanism.

If the measured weight does not increase sufficiently after opening the dispensing gate, the servo performs small repeated movements.

Open
 ↓
Check Weight
 ↓
No Significant Change
 ↓
Small Close / Open Movement
 ↓
Check Weight Again

This helps move food without requiring the user to manually shake the feeder.

---

## 3. Second Servo Gate – Food Release

After the required amount of food has been measured, a second servo-controlled gate releases the food.

The food falls from the measuring chamber onto the lower feeding tray/chute.

Measuring Chamber
        │
        ▼
 Second Servo Gate
        │
        ▼
   Feeding Chute
        │
        ▼
      Pet Bowl

This separates the measurement process from the final feeding process.

---

## 🐕 RFID Controlled Access

The Smart PAW system also includes RFID-based access control.

An RFID tag can be attached to the pet's collar.

When an authorized RFID tag is detected, the system can allow access to the food area.

This can help prevent other pets from accessing food intended for a particular pet.

---

## 🧠 System Working Principle

The complete feeding process is:

User Enters Required Food Weight
                │
                ▼
         ESP32 Starts Feeding
                │
                ▼
      First Servo Gate Opens
                │
                ▼
      Food Enters Measuring Chamber
                │
                ▼
        Load Cells Measure Weight
                │
                ▼
      Target Weight Reached?
          │             │
         NO            YES
          │             │
          ▼             ▼
 Continue Dispensing   Stop Servo
          │             │
          │             ▼
          │      Open Second Gate
          │             │
          └─────────────▼
                   Food Released
                         │
                         ▼
                      Pet Bowl

---

## 🔧 Main Hardware

| Component | Purpose |
|---|---|
| ESP32 | Main system controller |
| HX711 | Load cell signal amplifier |
| Load Cells | Measure food weight |
| MG996R Servo | Control food gates |
| PCA9685 | Servo motor controller |
| RFID Reader | Identify authorized pet |
| RFID Tag | Pet identification |
| 20×4 I2C LCD | Display system information |
| 4×4 Keypad | Enter target food weight |
| 5V Power Supply | Power the system |

---

## ⚙️ Main Mechanical Components

The CAD design includes:

- Main outer enclosure
- Upper food storage hopper
- Sloped food guide
- Food dispensing opening
- First servo mounting location
- Food measuring chamber
- Load-cell mounting area
- Second servo mounting location
- Food release mechanism
- Lower feeding chute
- Electronics mounting areas
- Front and side structural panels

---

## 💡 Design Goals

The mechanical design was developed with several goals:

- Compact construction
- Gravity-assisted food movement
- Controlled food dispensing
- Accurate food measurement
- Reduced food jamming
- Easy servo integration
- Easy electronics installation
- Simple assembly
- Low-cost construction
- Automated pet access

---

## 📂 Suggested Repository Structure

Smart-PAW-Mechanical-Design/
│
├── README.md
│
├── CAD/
│   ├── parts/
│   └── assembly/
│
├── STL/
│   └── printable-parts/
│
├── drawings/
│   └── technical-drawings/
│
├── images/
│   ├── smart-paw-design.jpg
│   ├── front-view.jpg
│   ├── side-view.jpg
│   └── assembly-view.jpg
│
└── docs/
    └── design-notes.md

The exact folders can be changed depending on which CAD, STL, STEP, or drawing files are available.

---

## 🚀 Future Improvements

Possible future improvements include:

- Improved food-flow geometry
- Modular removable food container
- Easier cleaning mechanism
- Improved servo gate design
- Fully 3D-printable components
- Additional load-cell support
- Improved RFID antenna placement
- Adjustable feeding tray
- Enclosure for electronic components
- Detection of low food level
- Mobile application integration
- Wi-Fi based feeding control
- Feeding history and statistics

---

## 🔗 Related Smart PAW Development

The mechanical design is only one part of the complete Smart PAW project.

The overall system combines:

Mechanical Design
       +
ESP32 Firmware
       +
Weight Measurement
       +
Servo Control
       +
RFID Identification
       +
User Interface
       =
   Smart PAW

---

## 🎯 Project Purpose

Smart PAW was developed as a practical embedded systems project that combines mechanical design, electronics, sensors, actuators, and software into a single automated pet feeding system.

The project demonstrates concepts including:

- Embedded system development
- ESP32 programming
- Sensor interfacing
- Load-cell measurement
- Closed-loop control
- Servo motor control
- RFID identification
- Mechanical CAD design
- Hardware/software integration

---

## 👨‍💻 Author

Pinsara Sarathchandra

Computer Engineering Undergraduate

---

## 📜 License

This project is intended for educational and research purposes.

If you plan to reuse or modify the design, please provide appropriate credit to the original project.
