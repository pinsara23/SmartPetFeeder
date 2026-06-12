// #include "HX711.h"

// #define DT 23
// #define SCK 26

// HX711 scale;

// // starting value
// float calibration_factor = -1015;

// void setup() {

//   Serial.begin(115200);

//   scale.begin(DT, SCK);

//   Serial.println();
//   Serial.println("HX711 Calibration");
//   Serial.println("-------------------");

//   Serial.println("Remove all weight from load cell");
//   delay(5000);

//   // set current reading as zero
//   scale.tare();

//   Serial.println("Tare completed");
//   Serial.println("Now place known weight");

// }

// void loop() {

//   // apply calibration factor
//   scale.set_scale(calibration_factor);

//   // average 10 readings
//   float weight = scale.get_units(10);

//   Serial.print("Weight: ");
//   Serial.print(weight);
//   Serial.println(" g");

//   Serial.print("Calibration Factor: ");
//   Serial.println(calibration_factor);

//   Serial.println("-------------------");

//   delay(1000);

//   // adjust factor from serial monitor
//   if (Serial.available()) {

//     char temp = Serial.read();

//     if (temp == '+') {
//       calibration_factor += 10;
//     }

//     else if (temp == '-') {
//       calibration_factor -= 10;
//     }
//   }
// }