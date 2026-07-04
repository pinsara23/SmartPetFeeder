![Smart pet feeding system](smart_paw.png)
<p align="center">
  <img src="smart_paw.png" alt="Smart Pet Feeder" width="500"/>
</p>

# Smart Pet Feeder

A smart, automated pet feeding system built using an **ESP32** microcontroller. This project uses a load cell for precise food measurement, servos for dispensing, and a keypad-driven LCD menu for scheduling and portion control. 

The firmware is developed using **PlatformIO** and the Arduino framework.

## Features
- **Real-Time Clock:** Setup current time on boot.
- **Precise Portion Control:** Input a target weight (in grams), and the system dispenses food using dual servos (food container servo and measure chamber servo) and an HX711 load cell amplifier.
- **Scheduled Feeding:** Set up to two distinct feeding times per day.
- **Interactive UI:** 20x4 I2C LCD and 4x4 keypad for an easy-to-use menu interface.
- **Automatic Dispensing Logic:** Automatically opens the dispensing chamber prior to scheduled feeding times and stops when the target weight is reached.

## Hardware Components
- ESP32 Development Board
- 20x4 I2C LCD Display
- 4x4 Matrix Keypad
- Load Cell + HX711 Amplifier
- 2x Servos (for the food container and measurement chamber)

## Pin Configuration (ESP32)

| Component             | Pins Used                   |
| --------------------- | --------------------------- |
| **Food Servo**        | GPIO 33                     |
| **Chamber Servo**     | GPIO 32                     |
| **Keypad Rows**       | GPIO 2, 0, 4, 16            |
| **Keypad Cols**       | GPIO 17, 5, 18, 19          |
| **HX711 (Load Cell)** | DT: GPIO 23, SCK: GPIO 26   |
| **LCD (I2C)**         | SDA, SCL (Default I2C pins) |

## Software Dependencies
The following libraries are required (automatically managed via `platformio.ini`):
- `marcoschwartz/LiquidCrystal_I2C`
- `chris--a/Keypad`
- `bogde/HX711`
- `madhephaestus/ESP32Servo`

## Usage / Menu Navigation

1. **Boot:** Enter the current time (HHMM) using the keypad and press `#` to confirm.
2. **Dashboard (Default Screen):** Displays Target Weight (TW), Current Weight (CW), Feed Times (F1 & F2), and the current Time.
3. **Menu System:** Press `C` from the dashboard to enter the inputs menu.
   - **Press A:** Set Target Weight (grams). Enter value and confirm with `#`.
   - **Press B:** Set Feed Times. Enter time for Feed 1 (HHMM), confirm with `#`, then enter Feed 2 (HHMM) and confirm with `#`.
   - **Press C:** Go back to the dashboard.
   - **Press *:** Reset all settings and values at any time.

## Calibration
Load cell calibration can be adjusted dynamically via the Serial Monitor (115200 baud).
- Send `+` to increase calibration factor by 10.
- Send `-` to decrease calibration factor by 10.
