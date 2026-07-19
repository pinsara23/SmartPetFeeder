// #include <Wire.h>
// #include <LiquidCrystal_I2C.h>
// #include <Keypad.h>
// #include "HX711.h"
// #include <ESP32Servo.h>

// Servo foodContainerServo;
// Servo measureChamberServo;

// const int foodContainerServoPin = 33;
// const int measureChamberServoPin = 32;

// // ---------------- Servo Angles ----------------
// int servoOpenAngle = 90;
// int servoCloseAngle = 0;
// int measureChamberOpenAngle = 90;
// int measureChamberCloseAngle = 0;


// // ---------------- LCD Configuration ----------------
// LiquidCrystal_I2C lcd(0x27, 20, 4);

// // ---------------- Keypad Configuration ----------------
// const byte ROWS = 4;
// const byte COLS = 4;

// char hexaKeys[ROWS][COLS] = {
//   {'1','2','3','A'},
//   {'4','5','6','B'},
//   {'7','8','9','C'},
//   {'*','0','#','D'}
// };

// byte rowPins[ROWS] = {2, 0, 4, 16};
// byte colPins[COLS] = {17, 5, 18, 19};

// Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

// // ---------------- HX711 Configuration ----------------
// #define DT 23
// #define SCK 26

// HX711 scale;

// // Calibration factor
// float calibration_factor = 1015;

// // ---------------- Time Variables ----------------
// int hours = 18;
// int minutes = 23;
// int seconds = 0;

// unsigned long lastUpdate = 0;

// // ---------------- Weight Variables ----------------
// String inputWeight = "";
// int targetWeight = 0;
// float currentWeightDisplay = 0.0;

// // ---------------- Feeding Time Variables ----------------
// // State machine for menu navigation
// enum MenuState {
//   INITIAL_TIME_SETUP, // Setup current time on boot
//   DEFAULT_SCREEN, // Main default dashboard
//   INPUTS_MENU,    // Show "A: Set Weight, B: Set Time"
//   WEIGHT_INPUT,   // Entering weight (numbers + # to confirm)
//   TIME_SLOT_1,    // Entering first feeding time (HHMM + # to confirm)
//   TIME_SLOT_2     // Entering second feeding time (HHMM + # to confirm)
// };

// MenuState currentState = INITIAL_TIME_SETUP;

// // Feeding time slots (stored as minutes from midnight for easy comparison)
// int feedTime1 = -1;  // -1 means not set
// int feedTime2 = -1;  // -1 means not set

// // Temporary variables for time input
// String inputTime = "";
// int timeInputStage = 0;  // 0 = entering hours, 1 = entering minutes
// int tempHours = 0;
// int tempMinutes = 0;

// // ---------------- Display Time Function ----------------
// void displayTimeOnLCD() {
//   if (currentState == DEFAULT_SCREEN || currentState == INITIAL_TIME_SETUP) return;

//   lcd.setCursor(0, 3);

//   if (hours < 10) lcd.print("0");
//   lcd.print(hours);

//   lcd.print(":");

//   if (minutes < 10) lcd.print("0");
//   lcd.print(minutes);

//   lcd.print(":");

//   if (seconds < 10) lcd.print("0");
//   lcd.print(seconds);

//   lcd.print("        ");
// }

// void updateDefaultScreen(float currentWeight) {
//   // Row 0
//   lcd.setCursor(0, 0);
//   lcd.print("TW:");
//   lcd.print(targetWeight);
//   lcd.print("g    "); // Clear old digits

//   lcd.setCursor(10, 0);
//   lcd.print("CW:");
//   lcd.print(currentWeight, 1);
//   lcd.print("g    "); // Clear old digits

//   // Row 1
//   lcd.setCursor(0, 1);
//   lcd.print("F1:");
//   if (feedTime1 >= 0) {
//     if (feedTime1 / 60 < 10) lcd.print("0");
//     lcd.print(feedTime1 / 60);
//     lcd.print(":");
//     if (feedTime1 % 60 < 10) lcd.print("0");
//     lcd.print(feedTime1 % 60);
//   } else {
//     lcd.print("--:--");
//   }
//   lcd.print("  ");

//   lcd.setCursor(10, 1);
//   lcd.print("F2:");
//   if (feedTime2 >= 0) {
//     if (feedTime2 / 60 < 10) lcd.print("0");
//     lcd.print(feedTime2 / 60);
//     lcd.print(":");
//     if (feedTime2 % 60 < 10) lcd.print("0");
//     lcd.print(feedTime2 % 60);
//   } else {
//     lcd.print("--:--");
//   }
//   lcd.print("  ");

//   // Row 2
//   lcd.setCursor(0, 2);
//   lcd.print("Time: ");
//   if (hours < 10) lcd.print("0"); lcd.print(hours); lcd.print(":");
//   if (minutes < 10) lcd.print("0"); lcd.print(minutes); lcd.print(":");
//   if (seconds < 10) lcd.print("0"); lcd.print(seconds);
//   lcd.print("      ");

//   // Row 3
//   lcd.setCursor(0, 3);
//   if (targetWeight > 0 && currentWeight > targetWeight) {
//     lcd.print("OVERLOAD! Press C   ");
//   } else {
//     lcd.print("Press C for inputs  ");
//   }
// }

// // ---------------- Helper Functions ----------------
// void showInitialTimeSetup() {
//   lcd.clear();
//   lcd.setCursor(0, 0);
//   lcd.print("Set Current Time:");
//   lcd.setCursor(0, 1);
//   lcd.print("HHMM: ");
//   lcd.print(inputTime);
//   lcd.print("      ");
//   lcd.setCursor(0, 3);
//   lcd.print("Press # to confirm");
// }

// void showDefaultScreen() {
//   updateDefaultScreen(currentWeightDisplay);
// }

// void showInputsMenu() {
//   lcd.clear();
//   lcd.setCursor(0, 0);
//   lcd.print("A: Set Weight");
//   lcd.setCursor(0, 1);
//   lcd.print("B: Set Feed Times");
//   lcd.setCursor(0, 2);
//   lcd.print("C: Back  *: Reset");
//   lcd.setCursor(0, 3);
//   displayTimeOnLCD();
// }

// void showWeightInput() {
//   lcd.clear();
//   lcd.setCursor(0, 0);
//   lcd.print("Enter Weight (g):");
//   lcd.setCursor(0, 1);
//   lcd.print("Input: ");
//   lcd.print(inputWeight);
//   lcd.print("g      ");
//   lcd.setCursor(0, 2);
//   lcd.print("Press # to confirm");
//   lcd.setCursor(0, 3);
//   displayTimeOnLCD();
// }

// void showTimeInput(int slot) {
//   lcd.clear();
//   lcd.setCursor(0, 0);
//   lcd.print("Feed Time ");
//   lcd.print(slot);
//   lcd.print(" (HHMM):");
//   lcd.setCursor(0, 1);
//   lcd.print("Input: ");
//   lcd.print(inputTime);
//   lcd.print("      ");
//   lcd.setCursor(0, 2);
//   lcd.print("Press # to confirm");
//   lcd.setCursor(0, 3);
//   displayTimeOnLCD();
// }

// void showTimeConfirmed(int slot, int h, int m) {
//   lcd.clear();
//   lcd.setCursor(0, 0);
//   lcd.print("Feed Time ");
//   lcd.print(slot);
//   lcd.print(" Set!");
//   lcd.setCursor(0, 1);
//   if (h < 10) lcd.print("0");
//   lcd.print(h);
//   lcd.print(":");
//   if (m < 10) lcd.print("0");
//   lcd.print(m);
//   lcd.setCursor(0, 3);
//   displayTimeOnLCD();
//   delay(1500);
// }

// void resetAll() {
//   inputWeight = "";
//   targetWeight = 0;
//   feedTime1 = -1;
//   feedTime2 = -1;
//   inputTime = "";
//   timeInputStage = 0;
//   tempHours = 0;
//   tempMinutes = 0;
//   currentState = DEFAULT_SCREEN;
  
//   lcd.clear();
//   lcd.setCursor(0, 0);
//   lcd.print("System Reset!");
//   lcd.setCursor(0, 1);
//   lcd.print("All values cleared");
//   delay(1500);
//   showDefaultScreen();
// }

// void checkFeedingTime() {
//   int currentMinutes = hours * 60 + minutes;
  
//   if (feedTime1 >= 0 && currentMinutes == feedTime1) {
//     lcd.setCursor(0, 0);
//     lcd.print("FEED TIME 1!      ");
//     Serial.println("FEED TIME 1 TRIGGERED!");
//   }
  
//   if (feedTime2 >= 0 && currentMinutes == feedTime2) {
//     lcd.setCursor(0, 0);
//     lcd.print("FEED TIME 2!      ");
//     Serial.println("FEED TIME 2 TRIGGERED!");
//   }
// }

// // urgent
// void handleDispensing(float currentWeight) {
//   int currentMinutes = hours * 60 + minutes;
//   bool shouldDispense = false;
//   bool measureChamberShouldOpen = false;
//   int targetAngle = servoCloseAngle;

//   // Check if within 2 hours (120 minutes) before feedTime1
//   if (feedTime1 >= 0) {
//     int diff1 = feedTime1 - currentMinutes;
//     if (diff1 < 0) diff1 += 24 * 60;
//     if (diff1 > 60 && diff1 <= 120) {
//       shouldDispense = true;
//     }
//     if (diff1 >= 30 && diff1 <= 55) {
//       measureChamberShouldOpen = true;
//     }
//   }
  
//   // Check if within 2 hours (120 minutes) before feedTime2
//   if (feedTime2 >= 0) {
//     int diff2 = feedTime2 - currentMinutes;
//     if (diff2 < 0) diff2 += 24 * 60;
//     if (diff2 > 60 && diff2 <= 120) {
//       shouldDispense = true;
//     }
//     if (diff2 >= 30 && diff2 <= 55) {
//       measureChamberShouldOpen = true;
//     }
//   }

//   if (shouldDispense && targetWeight > 0) {
//     float diffWeight = targetWeight - currentWeight;
    
//     if (diffWeight > 10.0) {
//       // Map remaining weight to servo angle. As weight rises, the angle approaches close.
//       if (targetWeight > 10) {
//         float proportion = (diffWeight - 10.0) / ((float)targetWeight - 10.0);
//         if (proportion > 1.0) proportion = 1.0;
//         if (proportion < 0.0) proportion = 0.0;
//         targetAngle = servoCloseAngle + (int)(proportion * (servoOpenAngle - servoCloseAngle));
//       } else {
//         targetAngle = servoOpenAngle;
//       }
//     } else {
//       // When 10g or less remaining, close completely
//       targetAngle = servoCloseAngle;
//     }
//   }

//   // Only update servo position when the angle changes to prevent jitter
//   static int currentServoAngle = -1;
//   if (targetAngle != currentServoAngle) {
//     foodContainerServo.write(targetAngle);
//     currentServoAngle = targetAngle;
//   }

//   // Handle measureChamberServo logic
//   int targetMeasureChamberAngle = measureChamberShouldOpen ? measureChamberOpenAngle : measureChamberCloseAngle;
//   static int currentMeasureChamberAngle = -1;
//   if (targetMeasureChamberAngle != currentMeasureChamberAngle) {
//     measureChamberServo.write(targetMeasureChamberAngle);
//     currentMeasureChamberAngle = targetMeasureChamberAngle;
//   }
// }

// void setup() {

//   Serial.begin(115200);

//   Wire.begin();

  
//   //servo attach
//   foodContainerServo.attach(foodContainerServoPin, 500, 2400); // Adjust pulse width range if needed
//   foodContainerServo.write(servoCloseAngle); // Initially closed

//   measureChamberServo.attach(measureChamberServoPin, 500, 2400); // Adjust pulse width range if needed
//   measureChamberServo.write(measureChamberCloseAngle); // Initially closed


//   // LCD Init
//   lcd.init();
//   lcd.backlight();
//   lcd.clear();

//   // HX711 Init
//   scale.begin(DT, SCK);

//   scale.tare();

//   delay(1000);

//   inputTime = "";
//   showInitialTimeSetup();
// }

// void loop() {
//   // ---------------- KEYPAD HANDLING ----------------
//   char key = customKeypad.getKey();

//   if (key) {
//     // Handle * (reset) in any state
//     if (key == '*') {
//       resetAll();
//       return;
//     }

//     // State machine for different modes
//     switch (currentState) {
//       case INITIAL_TIME_SETUP:
//         if (key >= '0' && key <= '9') {
//           if (inputTime.length() < 4) {
//             inputTime += key;
//           }
//           lcd.setCursor(6, 1);
//           lcd.print(inputTime);
//           lcd.print("      ");
//         }
//         else if (key == '#') {
//           if (inputTime.length() == 4) {
//             int h = inputTime.substring(0, 2).toInt();
//             int m = inputTime.substring(2, 4).toInt();
            
//             if (h >= 0 && h <= 23 && m >= 0 && m <= 59) {
//               hours = h;
//               minutes = m;
//               seconds = 0;
              
//               lcd.clear();
//               lcd.setCursor(0, 0);
//               lcd.print("Time Set!");
//               delay(1500);
              
//               inputTime = "";
//               currentState = DEFAULT_SCREEN;
//               showDefaultScreen();
//             }
//             else {
//               lcd.setCursor(0, 2);
//               lcd.print("Invalid Time!     ");
//               delay(1500);
//               lcd.setCursor(0, 2);
//               lcd.print("                  ");
//               inputTime = "";
//               lcd.setCursor(6, 1);
//               lcd.print("    ");
//             }
//           }
//           else {
//             lcd.setCursor(0, 2);
//             lcd.print("Enter 4 digits!   ");
//             delay(1500);
//             lcd.setCursor(0, 2);
//             lcd.print("                  ");
//             inputTime = "";
//             lcd.setCursor(6, 1);
//             lcd.print("    ");
//           }
//         }
//         break;

//       case DEFAULT_SCREEN:
//         if (key == 'C') {
//           currentState = INPUTS_MENU;
//           showInputsMenu();
//         }
//         break;

//       case INPUTS_MENU:
//         if (key == 'A') {
//           currentState = WEIGHT_INPUT;
//           inputWeight = "";
//           showWeightInput();
//         }
//         else if (key == 'B') {
//           currentState = TIME_SLOT_1;
//           inputTime = "";
//           timeInputStage = 0;
//           tempHours = 0;
//           tempMinutes = 0;
//           showTimeInput(1);
//         }
//         else if (key == 'C') {
//           currentState = DEFAULT_SCREEN;
//           showDefaultScreen();
//         }
//         break;

//       case WEIGHT_INPUT:
//         if (key >= '0' && key <= '9') {
//           if (inputWeight.length() < 4) {
//             inputWeight += key;
//           }
//           lcd.setCursor(0, 1);
//           lcd.print("Input: ");
//           lcd.print(inputWeight);
//           lcd.print("g      ");
//         }
//         else if (key == '#') {
//           int value = inputWeight.toInt();
//           if (value >= 1 && value <= 1000) {
//             targetWeight = value;
//             lcd.setCursor(0, 1);
//             lcd.print("Target: ");
//             lcd.print(targetWeight);
//             lcd.print("g      ");
//             Serial.print("Target Weight Set: ");
//             Serial.println(targetWeight);
//             delay(1500);
//           }
//           else {
//             lcd.setCursor(0, 1);
//             lcd.print("Invalid Range!    ");
//             delay(1500);
//           }
//           inputWeight = "";
//           currentState = DEFAULT_SCREEN;
//           showDefaultScreen();
//         }
//         break;

//       case TIME_SLOT_1:
//       case TIME_SLOT_2:
//         if (key >= '0' && key <= '9') {
//           if (inputTime.length() < 4) {
//             inputTime += key;
//           }
//           lcd.setCursor(0, 1);
//           lcd.print("Input: ");
//           lcd.print(inputTime);
//           lcd.print("      ");
//         }
//         else if (key == '#') {
//           if (inputTime.length() == 4) {
//             int h = inputTime.substring(0, 2).toInt();
//             int m = inputTime.substring(2, 4).toInt();
            
//             if (h >= 0 && h <= 23 && m >= 0 && m <= 59) {
//               if (currentState == TIME_SLOT_1) {
//                 feedTime1 = h * 60 + m;
//                 showTimeConfirmed(1, h, m);
//                 currentState = TIME_SLOT_2;
//                 inputTime = "";
//                 showTimeInput(2);
//               }
//               else {
//                 feedTime2 = h * 60 + m;
//                 showTimeConfirmed(2, h, m);
//                 currentState = DEFAULT_SCREEN;
//                 showDefaultScreen();
//               }
//             }
//             else {
//               lcd.setCursor(0, 1);
//               lcd.print("Invalid Time!     ");
//               delay(1500);
//               lcd.setCursor(0, 1);
//               lcd.print("Input: ");
//               lcd.print(inputTime);
//               lcd.print("      ");
//             }
//           }
//           else {
//             lcd.setCursor(0, 1);
//             lcd.print("Enter 4 digits!   ");
//             delay(1500);
//             lcd.setCursor(0, 1);
//             lcd.print("Input: ");
//             lcd.print(inputTime);
//             lcd.print("      ");
//           }
//           inputTime = "";
//         }
//         break;
//     }
//   }

//   // ---------------- Calibration Control ----------------
//   if (Serial.available()) {
//     char temp = Serial.read();
//     if (temp == '+') {
//       calibration_factor += 10;
//     }
//     else if (temp == '-') {
//       calibration_factor -= 10;
//     }
//   }

//   // ---------------- Update Every 1 Second ----------------
//   if (millis() - lastUpdate >= 1000) {
//     lastUpdate += 1000;

//     // ---------- Time Counter ----------
//     seconds++;
//     if (seconds >= 60) {
//       seconds = 0;
//       minutes++;
//       if (minutes >= 60) {
//         minutes = 0;
//         hours++;
//         if (hours >= 24) {
//           hours = 0;
//         }
//       }
//     }

//     // ---------- Read Weight ----------
//     scale.set_scale(calibration_factor);
//     float weight = scale.get_units(5);
//     if (weight < 0) {
//       weight = 0;
//     }
//     currentWeightDisplay = weight;

//     // ---------- Serial Output ----------
//     Serial.print("Weight: ");
//     Serial.print(weight);
//     Serial.print(" g | Target: ");
//     Serial.println(targetWeight);

//     // ---------- LCD Display Updates ----------
//     if (currentState == DEFAULT_SCREEN) {
//       updateDefaultScreen(weight);
//       if (targetWeight > 0 && weight > targetWeight) {
//         Serial.println("WARNING: Weight Exceeded!");
//       }
//     }

//     // ---------- Dispensing Logic ----------
//     handleDispensing(weight);

//     // ---------- Check Feeding Times ----------
//     checkFeedingTime();


//     // ---------- Display Time ----------
//     displayTimeOnLCD();
//   }
// }

