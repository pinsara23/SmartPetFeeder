![Smart pet feeding system](smart_paw.png)

# Smart Pet Feeder

A smart, automated pet feeding system built using an **ESP32** microcontroller. This project uses a load cell for precise food measurement, servos for dispensing, and a keypad-driven LCD menu for scheduling and portion control. 

The firmware is developed using **PlatformIO** and the Arduino framework.

## Features
- **Real-Time Clock:** Enter the current time in `HHMM` format during boot. The clock runs in firmware and wraps at midnight.
- **Precise Portion Control:** Enter a target weight from 1 to 1000 g. The HX711 load cell controls the food-container servo until the measured amount is within 3 g of the target.
- **Scheduled Feeding:** Configure two daily feeding times, `F1` and `F2`.
- **Two-Stage Feeding:** Food is first dispensed into the measuring chamber, then released into the bowl by a separate servo.
- **RFID Authentication:** An RFID scan authorizes early release during the configured release window. Release also occurs automatically at the exact scheduled minute as a fail-safe.
- **Jam Recovery:** If the measured weight does not increase by at least 0.5 g for 2 seconds, the food gate performs an automatic shake sequence before resuming.
- **Interactive UI:** A 20x4 I2C LCD and 4x4 keypad provide the setup, scheduling, weight, and reset controls.

## Hardware Components
- ESP32 Development Board
- 20x4 I2C LCD Display
- 4x4 Matrix Keypad
- Load Cell + HX711 Amplifier
- PCA9685 I2C PWM Servo Driver
- 2x Servos (for the food container and measurement chamber)
- MFRC522 RFID Reader

## Pin Configuration (ESP32)

| Component             | Pins Used                   |
| --------------------- | --------------------------- |
| **PCA9685 (Servos)**  | I2C (SDA, SCL)              |
| **Keypad Rows**       | GPIO 32, 33, 2, 4           |
| **Keypad Cols**       | GPIO 12, 13, 16, 17         |
| **HX711 (Load Cell)** | DT: GPIO 25, SCK: GPIO 26   |
| **RFID (MFRC522)**    | SS: GPIO 5, RST: GPIO 15    |
| **LCD (I2C)**         | SDA, SCL (Default I2C pins) |

## ESP32 Pinout

![ESP32 pinout](esp32_pinout.jpeg)

## Software Dependencies
The following libraries are required (automatically managed via `platformio.ini`):
- `marcoschwartz/LiquidCrystal_I2C`
- `chris--a/Keypad`
- `bogde/HX711`
- `adafruit/Adafruit PWM Servo Driver Library`
- `miguelbalboa/MFRC522`

## Usage / Menu Navigation

1. **Boot:** Enter the current time (HHMM) using the keypad and press `#` to confirm.
2. **Dashboard (Default Screen):** Displays Target Weight (TW), Current Weight (CW), Feed Times (F1 & F2), and the current Time.
3. **Menu System:** Press `C` from the dashboard to enter the inputs menu.
   - **Press A:** Set Target Weight (1-1000 g). Enter the value and confirm with `#`.
   - **Press B:** Set Feed Times. Enter Feed 1 and Feed 2 in `HHMM` format, confirming each with `#`. Hours must be `00-23` and minutes must be `00-59`.
   - **Press C:** Go back to the dashboard.
   - **Press *:** Reset all settings and values at any time.

## Current Feeding Logic

The firmware treats each scheduled feeding as two separate operations:

1. **Fill the measuring chamber:**
   - **Feed 1:** The food-container servo dispenses from 3 to 5 minutes before the scheduled time.
   - **Feed 2:** The food-container servo dispenses from 35 to 90 minutes before the scheduled time.
   - Outside the active window, or when no target weight is configured, the food gate remains closed.
2. **Release food into the bowl:**
   - **Feed 1:** The chamber can open during the 2 minutes before feeding when RFID is detected.
   - **Feed 2:** The chamber can open during the 30 minutes before feeding when RFID is detected.
   - Once RFID has been detected in the release window, authorization remains active until that feeding event. At the exact scheduled time, the chamber opens even without RFID.

The schedule calculations support crossing midnight. Each feeding time is compared with the current time as minutes from midnight, and a feeding time that has passed is treated as the next day's event. Each RFID authorization flag is cleared after its release window ends, so an old scan cannot authorize a later feeding.

### Portion and Servo Control

- While more than 5 g remains, the food gate uses the normal 18-degree opening.
- With approximately 3 to 5 g remaining, it uses 10-degree pulses. Each pulse stays open for 120 ms, then the system waits 700 ms for the load cell reading to settle.
- When the measured amount is within 3 g of the target, dispensing stops and the food gate closes.
- During jam recovery, the gate alternates between 5 and 40 degrees for up to 6 steps, with 180 ms between steps, then returns to normal dispensing.
- The measuring-chamber servo is closed at 0 degrees and opened at 100 degrees.
- If dispensing is not currently scheduled, the food gate is closed and jam tracking is reset.
- The dispensing state machine reports `IDLE`, `NORMAL FLOW`, `PRECISION PULSE`, `SETTLING`, or `COMPLETE` through the serial monitor.

Weight readings are refreshed approximately every 150 ms using two HX711 samples. Negative readings are clamped to zero. The LCD dashboard shows the target weight, current weight, both feed times, current time, and the current dispensing status. At each scheduled minute, the firmware also prints a feed-time trigger message to the serial monitor.

## Calibration
Load cell calibration can be adjusted dynamically via the Serial Monitor at 115200 baud. The initial calibration factor is 1015.
- Send `+` to increase the calibration factor by 10.
- Send `-` to decrease the calibration factor by 10.

## Reset Behavior

Press `*` from any menu to clear the target weight, both feed times, and temporary input. The food gate and measuring chamber close, the dispensing state is reset, and the dashboard is displayed.
