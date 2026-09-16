// #include "HX711.h"

// // HX711 #1
// #define DT1 23
// #define SCK1 26

// // HX711 #2
// #define DT2 19
// #define SCK2 18

// HX711 scale1;
// HX711 scale2;

// // Initial guesses
// float cal1 = -1015;
// float cal2 = -1015;

// void setup() {
//   Serial.begin(115200);

//   scale1.begin(DT1, SCK1);
//   scale2.begin(DT2, SCK2);

//   Serial.println("Remove all weight...");
//   delay(5000);

//   scale1.tare();
//   scale2.tare();

//   Serial.println("Tare completed.");
//   Serial.println();
//   Serial.println("Commands:");
//   Serial.println("q/a : increase/decrease cal1");
//   Serial.println("w/s : increase/decrease cal2");
// }

// void loop() {

//   scale1.set_scale(cal1);
//   scale2.set_scale(cal2);

//   float w1 = scale1.get_units(10);
//   float w2 = scale2.get_units(10);

//   float total = w1 + w2;

//   Serial.print("LC1: ");
//   Serial.print(w1, 2);
//   Serial.print(" g    ");

//   Serial.print("LC2: ");
//   Serial.print(w2, 2);
//   Serial.print(" g    ");

//   Serial.print("TOTAL: ");
//   Serial.print(total, 2);
//   Serial.println(" g");

//   Serial.print("Cal1 = ");
//   Serial.print(cal1);

//   Serial.print("   Cal2 = ");
//   Serial.println(cal2);

//   Serial.println("--------------------------------");

//   if (Serial.available()) {
//     char c = Serial.read();

//     switch (c) {
//       case 'q':
//         cal1 += 10;
//         break;

//       case 'a':
//         cal1 -= 10;
//         break;

//       case 'w':
//         cal2 += 10;
//         break;

//       case 's':
//         cal2 -= 10;
//         break;
//     }
//   }

//   delay(500);
// }