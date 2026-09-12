#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include "HX711.h"
#include <Adafruit_PWMServoDriver.h>
#include <SPI.h>
#include <MFRC522.h>


// ============================================================
// RFID
// ============================================================

const int RST_PIN = 15;
const int SS_PIN = 5;

MFRC522 mfrc522(SS_PIN, RST_PIN);


// ============================================================
// PCA9685 SERVO DRIVER
// ============================================================

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

const int foodContainerPin = 0;
const int measureChamberPin = 1;


// Convert servo angle to PCA9685 pulse
int angleToPulse(int angle)
{
    return map(angle, 0, 180, 102, 491);
}


// ============================================================
// FOOD CONTAINER SERVO SETTINGS
// ============================================================

const int FOOD_CLOSED_ANGLE = 0;

// Normal small opening
const int FOOD_NORMAL_ANGLE = 18;

// Small opening for final dispensing
const int FOOD_PULSE_ANGLE = 10;


// ============================================================
// ANTI-JAM SETTINGS
// ============================================================

const int SHAKE_OPEN_ANGLE = 40;
const int SHAKE_CLOSE_ANGLE = 5;

const unsigned long SHAKE_STEP_TIME = 180;

const int MAX_SHAKE_STEPS = 6;


// ============================================================
// MEASURING CHAMBER SERVO
// ============================================================

const int measureChamberOpenAngle = 100;
const int measureChamberCloseAngle = 0;


// ============================================================
// DISPENSING SETTINGS
// ============================================================

// Enter precision mode when 5 g remains
const float PRECISION_MARGIN = 5.0;


// IMPORTANT:
// If target-current <= 3g,
// consider the amount acceptable.
const float TARGET_TOLERANCE = 3.0;


// Precision pulse duration
const unsigned long PULSE_OPEN_TIME = 120;


// Wait for food/load cell to settle
const unsigned long SETTLE_TIME = 700;


// ============================================================
// JAM DETECTION
// ============================================================

const float MIN_WEIGHT_PROGRESS = 0.5;

const unsigned long JAM_DETECT_TIME = 2000;


float lastProgressWeight = 0.0;

unsigned long lastProgressTime = 0;


bool clearingJam = false;

int shakeStep = 0;

unsigned long shakeTimer = 0;


// ============================================================
// DISPENSING STATE
// ============================================================

enum DispenseState
{
    DISPENSE_IDLE,
    DISPENSE_NORMAL,
    DISPENSE_PULSE_OPEN,
    DISPENSE_SETTLING,
    DISPENSE_COMPLETE
};


DispenseState dispenseState = DISPENSE_IDLE;

unsigned long dispenseTimer = 0;


// ============================================================
// LCD
// ============================================================

LiquidCrystal_I2C lcd(0x27, 20, 4);


// ============================================================
// KEYPAD
// ============================================================

const byte ROWS = 4;
const byte COLS = 4;


char hexaKeys[ROWS][COLS] =
{
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};


byte rowPins[ROWS] =
{
    32, 33, 2, 4
};


byte colPins[COLS] =
{
    12, 13, 16, 17
};


Keypad customKeypad =
    Keypad(
        makeKeymap(hexaKeys),
        rowPins,
        colPins,
        ROWS,
        COLS
    );


// ============================================================
// HX711
// ============================================================

#define DT 25
#define SCK 26


HX711 scale;

float calibration_factor = 1015;


// ============================================================
// CLOCK
// ============================================================

int hours = 18;
int minutes = 23;
int seconds = 0;

unsigned long lastSecondUpdate = 0;


// ============================================================
// WEIGHT UPDATE
// ============================================================

unsigned long lastWeightUpdate = 0;

const unsigned long WEIGHT_UPDATE_INTERVAL = 150;


// ============================================================
// LCD UPDATE
// ============================================================

unsigned long lastLCDUpdate = 0;

const unsigned long LCD_UPDATE_INTERVAL = 500;


// ============================================================
// SERIAL UPDATE
// ============================================================

unsigned long lastSerialUpdate = 0;

const unsigned long SERIAL_UPDATE_INTERVAL = 1000;


// ============================================================
// WEIGHT VARIABLES
// ============================================================

String inputWeight = "";

int targetWeight = 0;

float currentWeightDisplay = 0.0;


// ============================================================
// MENU STATE
// ============================================================

enum MenuState
{
    INITIAL_TIME_SETUP,
    DEFAULT_SCREEN,
    INPUTS_MENU,
    WEIGHT_INPUT,
    TIME_SLOT_1,
    TIME_SLOT_2
};


MenuState currentState = INITIAL_TIME_SETUP;


// Feeding times in minutes from midnight
int feedTime1 = -1;
int feedTime2 = -1;


String inputTime = "";


// ============================================================
// FOOD SERVO
// ============================================================

void setFoodGate(int angle)
{
    static int previousAngle = -1;

    if (angle != previousAngle)
    {
        pwm.setPWM(
            foodContainerPin,
            0,
            angleToPulse(angle)
        );

        previousAngle = angle;

        Serial.print("Food Servo: ");
        Serial.print(angle);
        Serial.println(" deg");
    }
}


// ============================================================
// MEASURING CHAMBER SERVO
// ============================================================

void setMeasureChamber(int angle)
{
    static int previousAngle = -1;

    if (angle != previousAngle)
    {
        pwm.setPWM(
            measureChamberPin,
            0,
            angleToPulse(angle)
        );

        previousAngle = angle;

        Serial.print("Measure Servo: ");
        Serial.print(angle);
        Serial.println(" deg");
    }
}


// ============================================================
// DISPLAY TIME
// ============================================================

void displayTimeOnLCD()
{
    if (
        currentState == DEFAULT_SCREEN ||
        currentState == INITIAL_TIME_SETUP
    )
    {
        return;
    }


    lcd.setCursor(0, 3);


    if (hours < 10)
        lcd.print("0");

    lcd.print(hours);

    lcd.print(":");


    if (minutes < 10)
        lcd.print("0");

    lcd.print(minutes);

    lcd.print(":");


    if (seconds < 10)
        lcd.print("0");

    lcd.print(seconds);


    lcd.print("        ");
}


// ============================================================
// DEFAULT SCREEN
// ============================================================

void updateDefaultScreen(float currentWeight)
{
    // Row 0
    lcd.setCursor(0, 0);

    lcd.print("TW:");
    lcd.print(targetWeight);
    lcd.print("g    ");


    lcd.setCursor(10, 0);

    lcd.print("CW:");
    lcd.print(currentWeight, 1);
    lcd.print("g   ");


    // Row 1
    lcd.setCursor(0, 1);

    lcd.print("F1:");


    if (feedTime1 >= 0)
    {
        if ((feedTime1 / 60) < 10)
            lcd.print("0");

        lcd.print(feedTime1 / 60);

        lcd.print(":");

        if ((feedTime1 % 60) < 10)
            lcd.print("0");

        lcd.print(feedTime1 % 60);
    }
    else
    {
        lcd.print("--:--");
    }


    lcd.print("  ");


    lcd.setCursor(10, 1);

    lcd.print("F2:");


    if (feedTime2 >= 0)
    {
        if ((feedTime2 / 60) < 10)
            lcd.print("0");

        lcd.print(feedTime2 / 60);

        lcd.print(":");

        if ((feedTime2 % 60) < 10)
            lcd.print("0");

        lcd.print(feedTime2 % 60);
    }
    else
    {
        lcd.print("--:--");
    }


    // Row 2
    lcd.setCursor(0, 2);

    lcd.print("Time: ");


    if (hours < 10)
        lcd.print("0");

    lcd.print(hours);

    lcd.print(":");


    if (minutes < 10)
        lcd.print("0");

    lcd.print(minutes);

    lcd.print(":");


    if (seconds < 10)
        lcd.print("0");

    lcd.print(seconds);

    lcd.print("      ");


    // Row 3
    lcd.setCursor(0, 3);


    if (clearingJam)
    {
        lcd.print("Clearing food jam ");
    }

    else if (
        targetWeight > 0 &&
        currentWeight >= targetWeight - TARGET_TOLERANCE
    )
    {
        lcd.print("Food ready!        ");
    }

    else
    {
        lcd.print("Press C for inputs ");
    }
}


// ============================================================
// INITIAL TIME SCREEN
// ============================================================

void showInitialTimeSetup()
{
    lcd.clear();


    lcd.setCursor(0, 0);
    lcd.print("Set Current Time:");


    lcd.setCursor(0, 1);
    lcd.print("HHMM: ");
    lcd.print(inputTime);


    lcd.setCursor(0, 3);
    lcd.print("Press # to confirm");
}


// ============================================================
// DEFAULT SCREEN
// ============================================================

void showDefaultScreen()
{
    updateDefaultScreen(currentWeightDisplay);
}


// ============================================================
// INPUTS MENU
// ============================================================

void showInputsMenu()
{
    lcd.clear();


    lcd.setCursor(0, 0);
    lcd.print("A: Set Weight");


    lcd.setCursor(0, 1);
    lcd.print("B: Set Feed Times");


    lcd.setCursor(0, 2);
    lcd.print("C: Back  *: Reset");


    displayTimeOnLCD();
}


// ============================================================
// WEIGHT INPUT
// ============================================================

void showWeightInput()
{
    lcd.clear();


    lcd.setCursor(0, 0);
    lcd.print("Enter Weight (g):");


    lcd.setCursor(0, 1);
    lcd.print("Input: ");

    lcd.print(inputWeight);

    lcd.print("g      ");


    lcd.setCursor(0, 2);
    lcd.print("Press # to confirm");


    displayTimeOnLCD();
}


// ============================================================
// FEED TIME INPUT
// ============================================================

void showTimeInput(int slot)
{
    lcd.clear();


    lcd.setCursor(0, 0);

    lcd.print("Feed Time ");

    lcd.print(slot);

    lcd.print(" (HHMM):");


    lcd.setCursor(0, 1);

    lcd.print("Input: ");

    lcd.print(inputTime);

    lcd.print("      ");


    lcd.setCursor(0, 2);

    lcd.print("Press # to confirm");


    displayTimeOnLCD();
}


// ============================================================
// FEED TIME CONFIRM
// ============================================================

void showTimeConfirmed(int slot, int h, int m)
{
    lcd.clear();


    lcd.setCursor(0, 0);

    lcd.print("Feed Time ");

    lcd.print(slot);

    lcd.print(" Set!");


    lcd.setCursor(0, 1);


    if (h < 10)
        lcd.print("0");


    lcd.print(h);

    lcd.print(":");


    if (m < 10)
        lcd.print("0");


    lcd.print(m);


    delay(1000);
}


// ============================================================
// RESET DISPENSING
// ============================================================

void resetDispensingSystem()
{
    dispenseState = DISPENSE_IDLE;

    clearingJam = false;

    shakeStep = 0;

    lastProgressWeight = currentWeightDisplay;

    lastProgressTime = millis();

    setFoodGate(FOOD_CLOSED_ANGLE);
}


// ============================================================
// FULL RESET
// ============================================================

void resetAll()
{
    inputWeight = "";

    targetWeight = 0;

    feedTime1 = -1;
    feedTime2 = -1;

    inputTime = "";

    currentState = DEFAULT_SCREEN;


    resetDispensingSystem();


    setMeasureChamber(
        measureChamberCloseAngle
    );


    lcd.clear();


    lcd.setCursor(0, 0);

    lcd.print("System Reset!");


    lcd.setCursor(0, 1);

    lcd.print("Values cleared");


    delay(1000);


    showDefaultScreen();
}


// ============================================================
// CHECK FEED TIMES
// ============================================================

void checkFeedingTime()
{
    int currentMinutes =
        hours * 60 + minutes;


    if (
        feedTime1 >= 0 &&
        currentMinutes == feedTime1 &&
        seconds == 0
    )
    {
        Serial.println("FEED TIME 1 TRIGGERED!");
    }


    if (
        feedTime2 >= 0 &&
        currentMinutes == feedTime2 &&
        seconds == 0
    )
    {
        Serial.println("FEED TIME 2 TRIGGERED!");
    }
}


// ============================================================
// AUTOMATIC JAM CLEARING
// ============================================================

bool handleFoodJam(float currentWeight)
{
    // Food is moving
    if (
        currentWeight >=
        lastProgressWeight + MIN_WEIGHT_PROGRESS
    )
    {
        lastProgressWeight =
            currentWeight;


        lastProgressTime =
            millis();


        return false;
    }


    // Detect jam
    if (
        !clearingJam &&
        millis() - lastProgressTime >= JAM_DETECT_TIME
    )
    {
        Serial.println();

        Serial.println("========================");

        Serial.println("FOOD JAM DETECTED");

        Serial.println("Starting servo shake...");

        Serial.println("========================");


        clearingJam = true;

        shakeStep = 0;

        shakeTimer = millis();


        setFoodGate(
            SHAKE_CLOSE_ANGLE
        );


        return true;
    }


    // Anti-jam shake
    if (clearingJam)
    {
        if (
            millis() - shakeTimer >=
            SHAKE_STEP_TIME
        )
        {
            shakeTimer =
                millis();


            shakeStep++;


            if (shakeStep % 2 == 0)
            {
                setFoodGate(
                    SHAKE_OPEN_ANGLE
                );
            }

            else
            {
                setFoodGate(
                    SHAKE_CLOSE_ANGLE
                );
            }


            // Shake complete
            if (
                shakeStep >=
                MAX_SHAKE_STEPS
            )
            {
                clearingJam =
                    false;


                shakeStep =
                    0;


                setFoodGate(
                    FOOD_NORMAL_ANGLE
                );


                lastProgressWeight =
                    currentWeight;


                lastProgressTime =
                    millis();


                Serial.println(
                    "Jam clearing finished."
                );


                Serial.println(
                    "Returning to normal dispensing."
                );
            }
        }


        return true;
    }


    return false;
}


// ============================================================
// FOOD DISPENSING
// ============================================================

void controlFoodDispensing(
    float currentWeight,
    bool shouldDispense
)
{
    // --------------------------------------------------------
    // NOT DISPENSING
    // --------------------------------------------------------

    if (
        !shouldDispense ||
        targetWeight <= 0
    )
    {
        setFoodGate(
            FOOD_CLOSED_ANGLE
        );


        dispenseState =
            DISPENSE_IDLE;


        clearingJam =
            false;


        lastProgressWeight =
            currentWeight;


        lastProgressTime =
            millis();


        return;
    }


    float remainingWeight =
        targetWeight -
        currentWeight;


    // ========================================================
    // ACCEPTABLE WEIGHT
    //
    // Example:
    // Target 40g
    // Current 37g
    // Difference 3g
    //
    // STOP.
    // ========================================================

    if (
        remainingWeight <=
        TARGET_TOLERANCE
    )
    {
        setFoodGate(
            FOOD_CLOSED_ANGLE
        );


        if (
            dispenseState !=
            DISPENSE_COMPLETE
        )
        {
            Serial.println();

            Serial.println(
                "========================"
            );


            Serial.println(
                "FOOD AMOUNT ACCEPTED"
            );


            Serial.print(
                "Target: "
            );


            Serial.print(
                targetWeight
            );


            Serial.println(
                " g"
            );


            Serial.print(
                "Current: "
            );


            Serial.print(
                currentWeight,
                1
            );


            Serial.println(
                " g"
            );


            Serial.print(
                "Difference: "
            );


            Serial.print(
                remainingWeight,
                1
            );


            Serial.println(
                " g"
            );


            Serial.println(
                "========================"
            );
        }


        dispenseState =
            DISPENSE_COMPLETE;


        clearingJam =
            false;


        return;
    }


    // ========================================================
    // CHECK FOR JAM
    // ========================================================

    if (
        handleFoodJam(
            currentWeight
        )
    )
    {
        return;
    }


    // ========================================================
    // NORMAL SMALL-ANGLE DISPENSING
    // ========================================================

    if (
        remainingWeight >
        PRECISION_MARGIN
    )
    {
        if (
            dispenseState !=
            DISPENSE_NORMAL
        )
        {
            Serial.println(
                "Mode: NORMAL SMALL FLOW"
            );
        }


        dispenseState =
            DISPENSE_NORMAL;


        setFoodGate(
            FOOD_NORMAL_ANGLE
        );


        return;
    }


    // ========================================================
    // PRECISION DISPENSING
    //
    // This is only approximately 3-5g remaining.
    // Once <=3g, code above stops completely.
    // ========================================================

    switch (
        dispenseState
    )
    {
        // Start pulse
        case DISPENSE_IDLE:

        case DISPENSE_NORMAL:

        case DISPENSE_COMPLETE:
        {
            Serial.print(
                "Precision pulse. Remaining: "
            );


            Serial.print(
                remainingWeight,
                1
            );


            Serial.println(
                " g"
            );


            setFoodGate(
                FOOD_PULSE_ANGLE
            );


            dispenseTimer =
                millis();


            dispenseState =
                DISPENSE_PULSE_OPEN;


            break;
        }


        // Close pulse
        case DISPENSE_PULSE_OPEN:
        {
            if (
                millis() -
                dispenseTimer >=
                PULSE_OPEN_TIME
            )
            {
                setFoodGate(
                    FOOD_CLOSED_ANGLE
                );


                dispenseTimer =
                    millis();


                dispenseState =
                    DISPENSE_SETTLING;
            }


            break;
        }


        // Wait for food to settle
        case DISPENSE_SETTLING:
        {
            if (
                millis() -
                dispenseTimer >=
                SETTLE_TIME
            )
            {
                dispenseState =
                    DISPENSE_IDLE;


                lastProgressWeight =
                    currentWeight;


                lastProgressTime =
                    millis();
            }


            break;
        }
    }
}


// ============================================================
// RFID + FEEDING LOGIC
// ============================================================

void handleDispensing(
    float currentWeight
)
{
    int currentMinutes =
        hours * 60 + minutes;


    bool shouldDispense =
        false;


    bool measureChamberShouldOpen =
        false;


    // ========================================================
    // RFID DETECTION
    // ========================================================

    bool rfidDetected =
        false;


    if (
        mfrc522.PICC_IsNewCardPresent() &&
        mfrc522.PICC_ReadCardSerial()
    )
    {
        rfidDetected =
            true;


        Serial.println(
            "RFID detected."
        );


        mfrc522.PICC_HaltA();


        mfrc522.PCD_StopCrypto1();
    }


    static bool earlyRelease1 =
        false;


    static bool earlyRelease2 =
        false;


    // ========================================================
    // FEED TIME 1
    // ========================================================

    if (
        feedTime1 >= 0
    )
    {
        int diff1 =
            feedTime1 -
            currentMinutes;


        if (
            diff1 < 0
        )
        {
            diff1 +=
                24 * 60;
        }


        // Prepare food 3-5 min before
        if (
            diff1 >= 3 &&
            diff1 <= 5
        )
        {
            shouldDispense =
                true;
        }


        if (
            diff1 > 2
        )
        {
            earlyRelease1 =
                false;
        }


        if (
            diff1 >= 0 &&
            diff1 <= 2
        )
        {
            if (
                rfidDetected
            )
            {
                earlyRelease1 =
                    true;
            }


            if (
                earlyRelease1 ||
                diff1 == 0
            )
            {
                measureChamberShouldOpen =
                    true;
            }
        }
    }


    // ========================================================
    // FEED TIME 2
    // ========================================================

    if (
        feedTime2 >= 0
    )
    {
        int diff2 =
            feedTime2 -
            currentMinutes;


        if (
            diff2 < 0
        )
        {
            diff2 +=
                24 * 60;
        }


        if (
            diff2 >= 35 &&
            diff2 <= 90
        )
        {
            shouldDispense =
                true;
        }


        if (
            diff2 > 30
        )
        {
            earlyRelease2 =
                false;
        }


        if (
            diff2 >= 0 &&
            diff2 <= 30
        )
        {
            if (
                rfidDetected
            )
            {
                earlyRelease2 =
                    true;
            }


            if (
                earlyRelease2 ||
                diff2 == 0
            )
            {
                measureChamberShouldOpen =
                    true;
            }
        }
    }


    // ========================================================
    // FOOD CONTAINER -> MEASURING CHAMBER
    // ========================================================

    controlFoodDispensing(
        currentWeight,
        shouldDispense
    );


    // ========================================================
    // MEASURING CHAMBER -> BOWL
    // ========================================================

    if (
        measureChamberShouldOpen
    )
    {
        setMeasureChamber(
            measureChamberOpenAngle
        );
    }

    else
    {
        setMeasureChamber(
            measureChamberCloseAngle
        );
    }
}


// ============================================================
// WEIGHT UPDATE
// ============================================================

void updateWeight()
{
    if (
        millis() -
        lastWeightUpdate <
        WEIGHT_UPDATE_INTERVAL
    )
    {
        return;
    }


    lastWeightUpdate =
        millis();


    scale.set_scale(
        calibration_factor
    );


    float weight =
        scale.get_units(2);


    if (
        weight < 0
    )
    {
        weight =
            0;
    }


    currentWeightDisplay =
        weight;


    handleDispensing(
        currentWeightDisplay
    );
}


// ============================================================
// CLOCK UPDATE
// ============================================================

void updateClock()
{
    if (
        millis() -
        lastSecondUpdate <
        1000
    )
    {
        return;
    }


    lastSecondUpdate +=
        1000;


    seconds++;


    if (
        seconds >= 60
    )
    {
        seconds =
            0;


        minutes++;


        if (
            minutes >= 60
        )
        {
            minutes =
                0;


            hours++;


            if (
                hours >= 24
            )
            {
                hours =
                    0;
            }
        }
    }


    checkFeedingTime();
}


// ============================================================
// LCD UPDATE
// ============================================================

void updateLCD()
{
    if (
        millis() -
        lastLCDUpdate <
        LCD_UPDATE_INTERVAL
    )
    {
        return;
    }


    lastLCDUpdate =
        millis();


    if (
        currentState ==
        DEFAULT_SCREEN
    )
    {
        updateDefaultScreen(
            currentWeightDisplay
        );
    }

    else
    {
        displayTimeOnLCD();
    }
}


// ============================================================
// SERIAL MONITOR
// ============================================================

void updateSerialMonitor()
{
    if (
        millis() -
        lastSerialUpdate <
        SERIAL_UPDATE_INTERVAL
    )
    {
        return;
    }


    lastSerialUpdate =
        millis();


    Serial.print(
        "Weight: "
    );


    Serial.print(
        currentWeightDisplay,
        1
    );


    Serial.print(
        " g | Target: "
    );


    Serial.print(
        targetWeight
    );


    Serial.print(
        " g | "
    );


    switch (
        dispenseState
    )
    {
        case DISPENSE_IDLE:

            Serial.print(
                "IDLE"
            );

            break;


        case DISPENSE_NORMAL:

            Serial.print(
                "NORMAL FLOW"
            );

            break;


        case DISPENSE_PULSE_OPEN:

            Serial.print(
                "PRECISION PULSE"
            );

            break;


        case DISPENSE_SETTLING:

            Serial.print(
                "SETTLING"
            );

            break;


        case DISPENSE_COMPLETE:

            Serial.print(
                "COMPLETE"
            );

            break;
    }


    if (
        clearingJam
    )
    {
        Serial.print(
            " | ANTI-JAM"
        );
    }


    Serial.println();
}


// ============================================================
// SETUP
// ============================================================

void setup()
{
    Serial.begin(
        115200
    );


    // I2C
    Wire.begin();


    // PCA9685
    pwm.begin();


    pwm.setPWMFreq(
        50
    );


    delay(
        10
    );


    // Start closed
    setFoodGate(
        FOOD_CLOSED_ANGLE
    );


    setMeasureChamber(
        measureChamberCloseAngle
    );


    // LCD
    lcd.init();


    lcd.backlight();


    lcd.clear();


    // HX711
    scale.begin(
        DT,
        SCK
    );


    scale.set_scale(
        calibration_factor
    );


    scale.tare();


    // RFID
    SPI.begin();


    mfrc522.PCD_Init();


    delay(
        500
    );


    // Jam tracking
    lastProgressWeight =
        0;


    lastProgressTime =
        millis();


    // Initial screen
    inputTime =
        "";


    showInitialTimeSetup();


    Serial.println();

    Serial.println(
        "========================"
    );

    Serial.println(
        "SMART PET FEEDER READY"
    );

    Serial.println(
        "========================"
    );
}


// ============================================================
// MAIN LOOP
// ============================================================

void loop()
{
    // ========================================================
    // KEYPAD
    // ========================================================

    char key =
        customKeypad.getKey();


    if (
        key
    )
    {
        // Global reset
        if (
            key == '*'
        )
        {
            resetAll();

            return;
        }


        switch (
            currentState
        )
        {
            // =================================================
            // INITIAL TIME
            // =================================================

            case INITIAL_TIME_SETUP:
            {
                if (
                    key >= '0' &&
                    key <= '9'
                )
                {
                    if (
                        inputTime.length() <
                        4
                    )
                    {
                        inputTime +=
                            key;
                    }


                    lcd.setCursor(
                        6,
                        1
                    );


                    lcd.print(
                        inputTime
                    );


                    lcd.print(
                        "      "
                    );
                }


                else if (
                    key == '#'
                )
                {
                    if (
                        inputTime.length() ==
                        4
                    )
                    {
                        int h =
                            inputTime.substring(
                                0,
                                2
                            ).toInt();


                        int m =
                            inputTime.substring(
                                2,
                                4
                            ).toInt();


                        if (
                            h >= 0 &&
                            h <= 23 &&
                            m >= 0 &&
                            m <= 59
                        )
                        {
                            hours =
                                h;


                            minutes =
                                m;


                            seconds =
                                0;


                            lcd.clear();


                            lcd.setCursor(
                                0,
                                0
                            );


                            lcd.print(
                                "Time Set!"
                            );


                            delay(
                                1000
                            );


                            inputTime =
                                "";


                            currentState =
                                DEFAULT_SCREEN;


                            showDefaultScreen();
                        }

                        else
                        {
                            lcd.setCursor(
                                0,
                                2
                            );


                            lcd.print(
                                "Invalid Time!"
                            );


                            delay(
                                1000
                            );


                            inputTime =
                                "";


                            showInitialTimeSetup();
                        }
                    }

                    else
                    {
                        lcd.setCursor(
                            0,
                            2
                        );


                        lcd.print(
                            "Enter 4 digits!"
                        );


                        delay(
                            1000
                        );


                        inputTime =
                            "";


                        showInitialTimeSetup();
                    }
                }


                break;
            }


            // =================================================
            // DEFAULT SCREEN
            // =================================================

            case DEFAULT_SCREEN:
            {
                if (
                    key == 'C'
                )
                {
                    currentState =
                        INPUTS_MENU;


                    showInputsMenu();
                }


                break;
            }


            // =================================================
            // INPUT MENU
            // =================================================

            case INPUTS_MENU:
            {
                if (
                    key == 'A'
                )
                {
                    currentState =
                        WEIGHT_INPUT;


                    inputWeight =
                        "";


                    showWeightInput();
                }


                else if (
                    key == 'B'
                )
                {
                    currentState =
                        TIME_SLOT_1;


                    inputTime =
                        "";


                    showTimeInput(
                        1
                    );
                }


                else if (
                    key == 'C'
                )
                {
                    currentState =
                        DEFAULT_SCREEN;


                    showDefaultScreen();
                }


                break;
            }


            // =================================================
            // WEIGHT INPUT
            // =================================================

            case WEIGHT_INPUT:
            {
                if (
                    key >= '0' &&
                    key <= '9'
                )
                {
                    if (
                        inputWeight.length() <
                        4
                    )
                    {
                        inputWeight +=
                            key;
                    }


                    lcd.setCursor(
                        0,
                        1
                    );


                    lcd.print(
                        "Input: "
                    );


                    lcd.print(
                        inputWeight
                    );


                    lcd.print(
                        "g      "
                    );
                }


                else if (
                    key == '#'
                )
                {
                    int value =
                        inputWeight.toInt();


                    if (
                        value >= 1 &&
                        value <= 1000
                    )
                    {
                        targetWeight =
                            value;


                        resetDispensingSystem();


                        lcd.setCursor(
                            0,
                            1
                        );


                        lcd.print(
                            "Target: "
                        );


                        lcd.print(
                            targetWeight
                        );


                        lcd.print(
                            "g      "
                        );


                        Serial.print(
                            "Target set: "
                        );


                        Serial.print(
                            targetWeight
                        );


                        Serial.println(
                            " g"
                        );


                        delay(
                            1000
                        );
                    }

                    else
                    {
                        lcd.setCursor(
                            0,
                            1
                        );


                        lcd.print(
                            "Invalid Range!"
                        );


                        delay(
                            1000
                        );
                    }


                    inputWeight =
                        "";


                    currentState =
                        DEFAULT_SCREEN;


                    showDefaultScreen();
                }


                break;
            }


            // =================================================
            // FEED TIMES
            // =================================================

            case TIME_SLOT_1:

            case TIME_SLOT_2:
            {
                if (
                    key >= '0' &&
                    key <= '9'
                )
                {
                    if (
                        inputTime.length() <
                        4
                    )
                    {
                        inputTime +=
                            key;
                    }


                    lcd.setCursor(
                        0,
                        1
                    );


                    lcd.print(
                        "Input: "
                    );


                    lcd.print(
                        inputTime
                    );


                    lcd.print(
                        "      "
                    );
                }


                else if (
                    key == '#'
                )
                {
                    if (
                        inputTime.length() ==
                        4
                    )
                    {
                        int h =
                            inputTime.substring(
                                0,
                                2
                            ).toInt();


                        int m =
                            inputTime.substring(
                                2,
                                4
                            ).toInt();


                        if (
                            h >= 0 &&
                            h <= 23 &&
                            m >= 0 &&
                            m <= 59
                        )
                        {
                            if (
                                currentState ==
                                TIME_SLOT_1
                            )
                            {
                                feedTime1 =
                                    h * 60 + m;


                                showTimeConfirmed(
                                    1,
                                    h,
                                    m
                                );


                                currentState =
                                    TIME_SLOT_2;


                                inputTime =
                                    "";


                                showTimeInput(
                                    2
                                );
                            }

                            else
                            {
                                feedTime2 =
                                    h * 60 + m;


                                showTimeConfirmed(
                                    2,
                                    h,
                                    m
                                );


                                currentState =
                                    DEFAULT_SCREEN;


                                inputTime =
                                    "";


                                showDefaultScreen();
                            }
                        }

                        else
                        {
                            lcd.setCursor(
                                0,
                                1
                            );


                            lcd.print(
                                "Invalid Time!"
                            );


                            delay(
                                1000
                            );


                            inputTime =
                                "";


                            if (
                                currentState ==
                                TIME_SLOT_1
                            )
                            {
                                showTimeInput(
                                    1
                                );
                            }

                            else
                            {
                                showTimeInput(
                                    2
                                );
                            }
                        }
                    }

                    else
                    {
                        lcd.setCursor(
                            0,
                            1
                        );


                        lcd.print(
                            "Enter 4 digits!"
                        );


                        delay(
                            1000
                        );


                        inputTime =
                            "";


                        if (
                            currentState ==
                            TIME_SLOT_1
                        )
                        {
                            showTimeInput(
                                1
                            );
                        }

                        else
                        {
                            showTimeInput(
                                2
                            );
                        }
                    }
                }


                break;
            }
        }
    }


    // ========================================================
    // SERIAL HX711 CALIBRATION
    // ========================================================

    if (
        Serial.available()
    )
    {
        char temp =
            Serial.read();


        if (
            temp == '+'
        )
        {
            calibration_factor +=
                10;


            Serial.print(
                "Calibration: "
            );


            Serial.println(
                calibration_factor
            );
        }


        else if (
            temp == '-'
        )
        {
            calibration_factor -=
                10;


            Serial.print(
                "Calibration: "
            );


            Serial.println(
                calibration_factor
            );
        }
    }


    // ========================================================
    // SYSTEM UPDATES
    // ========================================================

    updateClock();

    updateWeight();

    updateLCD();

    updateSerialMonitor();
}