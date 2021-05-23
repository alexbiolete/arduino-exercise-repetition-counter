#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Pinout setup
#define PIN_ULTRASONIC_PING  4
#define PIN_ULTRASONIC_ECHO  2
#define PIN_BUZZER           17
#define PIN_LED              16
#define PIN_BUTTON           15

// Display dimensions
#define SCREEN_WIDTH         128 // OLED display width, in pixels
#define SCREEN_HEIGHT        64  // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET           -1 // Reset pin # (or -1 if sharing Arduino reset pin)

// Device mode variables
#define MODE_PULLUP_DISTANCE 50
#define MODE_PUSHUP_OFFSET   50

// String constants
#define STRING_SEPARATOR     "--------------------"
#define STRING_EXERCISE      "Exercise: "
#define STRING_PULLUPS       "Pullups"
#define STRING_PUSHUPS       "Pushups"
#define STRING_REPETITIONS   "Repetitions: "

// Display initialization
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Exercise initialization
struct Exercise {
  int totalRepetitions = 0;
  bool repetitionIsValid = false;
} exercise;

// Function to compute distance from microseconds to centimeters
unsigned long microsecondsToCentimeters(unsigned long microseconds) {
  return microseconds / 29 / 2;
}

// Code that is ran once
void setup() {
  // Open serial port, set data rate (bauds per second)
  Serial.begin(9600);

  // Pinout configuration
  pinMode(PIN_ULTRASONIC_PING, OUTPUT);
  pinMode(PIN_ULTRASONIC_ECHO, INPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_BUTTON, INPUT);

  // Show data on terminal
  Serial.println(STRING_SEPARATOR);
  Serial.print(STRING_EXERCISE);
  Serial.println(STRING_PULLUPS);
  Serial.print(STRING_REPETITIONS);
  Serial.println(exercise.totalRepetitions);

  // OLED display setup
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for ( ; ; ); // Don't proceed, loop forever
  }

  // Clear display buffer
  display.clearDisplay();

  // Display settings
  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(WHITE); // Draw white text
  display.setCursor(0, 0);     // Start at top-left corner
  display.cp437(true);         // Use full 256 char 'Code Page 437' font

  // Show data on display
  display.print(STRING_EXERCISE);
  display.println(STRING_PULLUPS);
  display.print(STRING_REPETITIONS);
  display.println(exercise.totalRepetitions);
  display.display();
}

// Code that is ran repeatedly
void loop() {
  unsigned long durationInMicroseconds, distanceInCentimeters;

  // Send and receive ultrasound to detect distance
  digitalWrite(PIN_ULTRASONIC_PING, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_ULTRASONIC_PING, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_ULTRASONIC_PING, LOW);
  durationInMicroseconds = pulseIn(PIN_ULTRASONIC_ECHO, HIGH);
  // Convert to centimeters
  distanceInCentimeters = microsecondsToCentimeters(durationInMicroseconds);

  // Check if distance in centimeters is less than MODE_PULLUP_DISTANCE from the sensor
  if (distanceInCentimeters > MODE_PULLUP_DISTANCE && exercise.repetitionIsValid) {
    exercise.repetitionIsValid = false;
  } else if (distanceInCentimeters <= MODE_PULLUP_DISTANCE && !exercise.repetitionIsValid) {
    // If user is in range, count the repetition as valid
    exercise.repetitionIsValid = true;
    exercise.totalRepetitions++;

    // When counting a repetition, make a sound and turn on the LED for a short time
    digitalWrite(PIN_BUZZER, HIGH);
    digitalWrite(PIN_LED, HIGH);
    delay(100);
    digitalWrite(PIN_BUZZER, LOW);
    digitalWrite(PIN_LED, LOW);
    delay(50);

    // Show data on terminal
    Serial.println(STRING_SEPARATOR);
    Serial.print(STRING_EXERCISE);
    Serial.println(STRING_PULLUPS);
    Serial.print(STRING_REPETITIONS);
    Serial.println(exercise.totalRepetitions);

    // OLED display update
    // Clear display buffer
    display.clearDisplay();

    // Display settings
    display.setTextSize(1);      // Normal 1:1 pixel scale
    display.setTextColor(WHITE); // Draw white text
    display.setCursor(0, 0);     // Start at top-left corner
    display.cp437(true);         // Use full 256 char 'Code Page 437' font

    // Show data on display
    display.print(STRING_EXERCISE);
    display.println(STRING_PULLUPS);
    display.print(STRING_REPETITIONS);
    display.println(exercise.totalRepetitions);
    display.display();
  }

  // Short delay after each iteration
  delay(300);
}
