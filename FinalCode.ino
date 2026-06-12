#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include "HX711.h"

// ---------------- LCD Configuration ----------------
LiquidCrystal_I2C lcd(0x27, 20, 4);

// ---------------- Keypad Configuration ----------------
const byte ROWS = 4;
const byte COLS = 4;

char hexaKeys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {2, 0, 4, 16};
byte colPins[COLS] = {17, 5, 18, 19};

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

// ---------------- HX711 Configuration ----------------
#define DT 23
#define SCK 26

HX711 scale;

// Calibration factor
float calibration_factor = -1015;

// ---------------- Time Variables ----------------
int hours = 18;
int minutes = 23;
int seconds = 0;

unsigned long lastUpdate = 0;

// ---------------- Weight Variables ----------------
String inputWeight = "";
int targetWeight = 0;

void setup() {

  Serial.begin(115200);

  Wire.begin();

  // LCD Init
  lcd.init();
  lcd.backlight();
  lcd.clear();

  // HX711 Init
  scale.begin(DT, SCK);

  scale.tare();

  delay(1000);

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Press A");

  lcd.setCursor(0,1);
  lcd.print("to enter the weight");
  
}

void loop() {

  // ---------------- KEYPAD HANDLING ----------------
  char key = customKeypad.getKey();

  if (key) {

    // ---------- Number Input ----------
    if (key >= '0' && key <= '9') {

      // Allow max 4 digits
      if (inputWeight.length() < 4) {
        inputWeight += key;
      }

      lcd.setCursor(0, 1);
      lcd.print("Input: ");
      lcd.print(inputWeight);
      lcd.print("g      ");
    }

    // ---------- Confirm Weight ----------
    else if (key == '#') {

      int value = inputWeight.toInt();

      // Allow only 1g - 1000g
      if (value >= 1 && value <= 1000) {

        targetWeight = value;

        lcd.setCursor(0, 1);
        lcd.print("Target: ");
        lcd.print(targetWeight);
        lcd.print("g      ");

        Serial.print("Target Weight Set: ");
        Serial.println(targetWeight);
      }
      else {

        lcd.setCursor(0, 1);
        lcd.print("Invalid Range!    ");

        delay(1500);

        lcd.setCursor(0, 1);
        lcd.print("                    ");
      }

      inputWeight = "";
    }

    // ---------- Reset using * ----------
    else if (key == '*') {

      // Reset values
      inputWeight = "";
      targetWeight = 0;

      lcd.setCursor(0, 1);
      lcd.print("Weight Reset      ");

      lcd.setCursor(0, 2);
      lcd.print("W:0g RESET        ");

      lcd.setCursor(0, 0);
      lcd.print("System Reset      ");

      Serial.println("Target Weight Reset!");

      delay(1500);

      lcd.clear();

      lcd.setCursor(0, 0);
      lcd.print("Enter Weight(g):");
    }
  }

  // ---------------- Calibration Control ----------------
  if (Serial.available()) {

    char temp = Serial.read();

    if (temp == '+') {
      calibration_factor += 10;
    }

    else if (temp == '-') {
      calibration_factor -= 10;
    }
  }

  // ---------------- Update Every 1 Second ----------------
  if (millis() - lastUpdate >= 1000) {

    lastUpdate += 1000;

    // ---------- Time Counter ----------
    seconds++;

    if (seconds >= 60) {

      seconds = 0;
      minutes++;

      if (minutes >= 60) {

        minutes = 0;
        hours++;

        if (hours >= 24) {
          hours = 0;
        }
      }
    }

    // ---------- Read Weight ----------
    scale.set_scale(calibration_factor);

    float weight = scale.get_units(5);

    // Remove negative tiny noise
    if (weight < 0) {
      weight = 0;
    }

    // ---------- Serial Output ----------
    Serial.print("Weight: ");
    Serial.print(weight);
    Serial.print(" g | Target: ");
    Serial.println(targetWeight);

    // ---------- LCD Weight Display ----------
    lcd.setCursor(0, 2);

    lcd.print("W:");
    lcd.print(weight, 1);
    lcd.print("g ");

    // ---------- Overload Check ----------
    if (targetWeight > 0 && weight > targetWeight) {

      lcd.print("OVERLOAD!");

      // Warning message
      lcd.setCursor(0, 0);
      lcd.print("WARNING EXCEEDED!");

      Serial.println("WARNING: Weight Exceeded!");
    }

    else {

      lcd.print("NORMAL     ");

      // Restore heading
      lcd.setCursor(0, 0);
      lcd.print("Enter Weight(g):");
    }

    // ---------- Display Time ----------
    displayTimeOnLCD();
  }
}

// ---------------- Display Time Function ----------------
void displayTimeOnLCD() {

  lcd.setCursor(0, 3);

  if (hours < 10) lcd.print("0");
  lcd.print(hours);

  lcd.print(":");

  if (minutes < 10) lcd.print("0");
  lcd.print(minutes);

  lcd.print(":");

  if (seconds < 10) lcd.print("0");
  lcd.print(seconds);

  lcd.print("        ");
}