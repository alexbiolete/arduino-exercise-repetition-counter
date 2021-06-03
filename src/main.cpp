#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// #include <BLEDevice.h>
// #include <BLEUtils.h>
// #include <BLEServer.h>

#define SERVICE_UUID                       "8563fc8b-1ff0-4d52-af8c-6f3cafb4a640"
#define CHARACTERISTIC_UUID                "951aebbe-cea5-4ecb-9075-944b47fc07d8"

// Pinout setup
#define PIN_ULTRASONIC_PING                4
#define PIN_ULTRASONIC_ECHO                2
#define PIN_BUZZER                         17
#define PIN_LED                            16
#define PIN_BUTTON                         15

// Display dimensions
#define SCREEN_WIDTH                       128 // OLED display width, in pixels
#define SCREEN_HEIGHT                      64  // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET                         -1 // Reset pin # (or -1 if sharing Arduino reset pin)

// Device mode variables
#define MODE_PULLUP_DISTANCE               50
#define MODE_PUSHUP_NEGATIVE_MINIMUM       10
#define MODE_PUSHUP_POSITIVE_LOWER_LIMIT   20
#define MODE_PUSHUP_POSITIVE_UPPER_LIMIT   30

// String constants
#define STRING_SEPARATOR                   "--------------------"
#define STRING_EXERCISE                    "Exercise: "
#define STRING_PULLUPS                     "PULLUPS"
#define STRING_PUSHUPS                     "PUSHUPS"
#define STRING_SETS                        "Sets: "
#define STRING_REPETITIONS                 "Reps.: "

// Time related variables
#define ITERATION_DELAY                    500
#define MAX_REP_COOLDOWN                   5000

// Display initialization
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Exercise initialization
struct Exercise {
  String name = STRING_PUSHUPS;
  int totalSets = 0;
  int totalRepetitions = 0;
  int cooldownTime = 0;             // Time based on which we notice if the set was finished (> 5s).
  bool validatedPullup = false;
  bool validatedPositivePushupMotion = false, validatedNegativePushupMotion = false;
} exercise;

// Function to compute distance from microseconds to centimeters
unsigned long microsecondsToCentimeters(unsigned long microseconds) {
  return (microseconds / 29.155) / 2;
}

void reinitializeExercise() {
    exercise.totalSets = 0;
    exercise.totalRepetitions = 0;
    exercise.cooldownTime = 0;
    exercise.validatedPullup = false;
    exercise.validatedPositivePushupMotion = false;
    exercise.validatedNegativePushupMotion = false;
}

void notifyStart() {
  // When starting a set, make some sounds and flash the LED a few times.
  digitalWrite(PIN_BUZZER, HIGH);
  digitalWrite(PIN_LED, HIGH);
  delay(100);
  digitalWrite(PIN_BUZZER, LOW);
  digitalWrite(PIN_LED, LOW);
  delay(375);
  digitalWrite(PIN_BUZZER, HIGH);
  digitalWrite(PIN_LED, HIGH);
  delay(100);
  digitalWrite(PIN_BUZZER, LOW);
  digitalWrite(PIN_LED, LOW);
  delay(375);
  digitalWrite(PIN_BUZZER, HIGH);
  digitalWrite(PIN_LED, HIGH);
  delay(100);
  digitalWrite(PIN_BUZZER, LOW);
  digitalWrite(PIN_LED, LOW);
  delay(375);
  digitalWrite(PIN_BUZZER, HIGH);
  digitalWrite(PIN_LED, HIGH);
  delay(250);
  digitalWrite(PIN_BUZZER, LOW);
  digitalWrite(PIN_LED, LOW);
  delay(375);
}

void notifyChange() {
  // When switching exercises, make a sound and turn on the LED for a short time
  digitalWrite(PIN_BUZZER, HIGH);
  digitalWrite(PIN_LED, HIGH);
  delay(50);
  digitalWrite(PIN_BUZZER, LOW);
  digitalWrite(PIN_LED, LOW);
  delay(25);
}

void notifyCount() {
  // When counting a repetition, make a sound and turn on the LED for a short time
  digitalWrite(PIN_BUZZER, HIGH);
  digitalWrite(PIN_LED, HIGH);
  delay(100);
  digitalWrite(PIN_BUZZER, LOW);
  digitalWrite(PIN_LED, LOW);
  delay(50);
}

void notifyEnd() {
  // When ending a set, make some sounds and flash the LED a few times.
  digitalWrite(PIN_BUZZER, HIGH);
  digitalWrite(PIN_LED, HIGH);
  delay(100);
  digitalWrite(PIN_BUZZER, LOW);
  digitalWrite(PIN_LED, LOW);
  delay(75);
  digitalWrite(PIN_BUZZER, HIGH);
  digitalWrite(PIN_LED, HIGH);
  delay(100);
  digitalWrite(PIN_BUZZER, LOW);
  digitalWrite(PIN_LED, LOW);
  delay(75);
}

void printExerciseInformation() {
  // Show data on terminal
  Serial.println(STRING_SEPARATOR);
  Serial.print(STRING_EXERCISE);
  Serial.println(exercise.name);
  Serial.print(STRING_SETS);
  Serial.println(exercise.totalSets);
  Serial.print(STRING_REPETITIONS);
  Serial.println(exercise.totalRepetitions);
}

void displayExerciseInformation() {
    // OLED display update
    // Clear display buffer
    display.clearDisplay();

    // Display settings
    display.setTextSize(2);      // Normal 1:1 pixel scale
    display.setTextColor(WHITE); // Draw white text
    display.setCursor(0, 0);     // Start at top-left corner
    display.cp437(true);         // Use full 256 char 'Code Page 437' font

    // Show data on display
    display.print(STRING_EXERCISE);
    display.println(exercise.name);
    display.print(STRING_SETS);
    display.println(exercise.totalSets);
    display.print(STRING_REPETITIONS);
    display.println(exercise.totalRepetitions);
    display.display();
}

// Code that is ran once
void setup() {
  // Open serial port, set data rate (bauds per second)
  Serial.begin(115200);

  // BEGIN BLE TEST CODE
  // Serial.println("Starting BLE work!");

  // BLEDevice::init("Prototype ERC");
  // BLEServer *pServer = BLEDevice::createServer();
  // BLEService *pService = pServer->createService(SERVICE_UUID);
  // BLECharacteristic *pCharacteristic = pService->createCharacteristic(
  //   CHARACTERISTIC_UUID,
  //   BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE
  // );

  // pCharacteristic->setValue("Hello World says Neil");
  // pService->start();
  // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
  // BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  // pAdvertising->addServiceUUID(SERVICE_UUID);
  // pAdvertising->setScanResponse(true);
  // pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  // pAdvertising->setMinPreferred(0x12);
  // BLEDevice::startAdvertising();
  // Serial.println("Characteristic defined! Now you can read it in your phone!");
  // END BLE TEST CODE

  // Pinout configuration
  pinMode(PIN_ULTRASONIC_PING, OUTPUT);
  pinMode(PIN_ULTRASONIC_ECHO, INPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_BUTTON, INPUT_PULLUP);

  notifyStart();
  printExerciseInformation();

  // OLED display setup
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for ( ; ; ); // Don't proceed, loop forever
  }

  displayExerciseInformation();
}

// Code that is ran repeatedly
void loop() {
  if (digitalRead(PIN_BUTTON) == LOW && exercise.name == STRING_PUSHUPS) {
    exercise.name = STRING_PULLUPS;
    reinitializeExercise();

    notifyChange();
    printExerciseInformation();
    displayExerciseInformation();
  } else if (digitalRead(PIN_BUTTON) == LOW && exercise.name == STRING_PULLUPS) {
    exercise.name = STRING_PUSHUPS;
    reinitializeExercise();

    notifyChange();
    printExerciseInformation();
    displayExerciseInformation();
  }

  // Send and receive ultrasound to detect distance
  digitalWrite(PIN_ULTRASONIC_PING, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_ULTRASONIC_PING, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_ULTRASONIC_PING, LOW);
  pinMode(PIN_ULTRASONIC_ECHO, INPUT);
  unsigned long durationInMicroseconds = pulseIn(PIN_ULTRASONIC_ECHO, HIGH);
  // Convert to centimeters
  unsigned long distanceInCentimeters = microsecondsToCentimeters(durationInMicroseconds);

  if (exercise.name == STRING_PUSHUPS) {
    if (distanceInCentimeters < MODE_PUSHUP_NEGATIVE_MINIMUM) {
      exercise.validatedNegativePushupMotion = true;
    } else if (exercise.validatedNegativePushupMotion == true
      && distanceInCentimeters > MODE_PUSHUP_POSITIVE_LOWER_LIMIT
      && distanceInCentimeters <= MODE_PUSHUP_POSITIVE_UPPER_LIMIT) {
      exercise.validatedPositivePushupMotion = true;
    }

    if (exercise.validatedPositivePushupMotion == true && exercise.validatedNegativePushupMotion == true) {
      exercise.validatedNegativePushupMotion = false;
      exercise.validatedPositivePushupMotion = false;
      exercise.totalRepetitions++;
      exercise.cooldownTime = 0;

      notifyCount();
      printExerciseInformation();
      displayExerciseInformation();
    }
  } else if (exercise.name == STRING_PULLUPS) {
    // Check if distance in centimeters is less than MODE_PULLUP_DISTANCE from the sensor
    if (distanceInCentimeters > MODE_PULLUP_DISTANCE && exercise.validatedPullup) {
      exercise.validatedPullup = false;
    } else if (distanceInCentimeters <= MODE_PULLUP_DISTANCE && !exercise.validatedPullup) {
      // If user is in range, count the repetition as valid
      exercise.validatedPullup = true;
      exercise.totalRepetitions++;
      exercise.cooldownTime = 0;

      notifyCount();
      printExerciseInformation();
      displayExerciseInformation();
    }
  }

  // If max. cooldown between reps. is exceeded, start a new set.
  if (exercise.cooldownTime >= MAX_REP_COOLDOWN && exercise.totalRepetitions != 0) {
    exercise.totalSets++;
    exercise.totalRepetitions = 0;
    exercise.cooldownTime = 0;

    notifyEnd();
    printExerciseInformation();
    displayExerciseInformation();
  }

  // Short delay after each iteration
  exercise.cooldownTime += ITERATION_DELAY;
  delay(ITERATION_DELAY);
}
