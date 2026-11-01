#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <LiquidCrystal_I2C.h>
#include <math.h>

// Components
Adafruit_MPU6050 mpu;
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Pin connections
const int redLED = 7;
const int greenLED = 6;
const int buzzer = 9;
const int buttonPin = 2;

// Simulated GPS coordinates
float latitude = 14.4426;
float longitude = 79.9865;

// Accident detection threshold (m/s^2)
const float accidentThreshold = 25.0;

// Alert timing
const unsigned long alertDuration = 5000;
const unsigned long cooldown = 5000;

bool alertActive = false;
bool hasAlerted = false;

unsigned long alertStartTime = 0;
unsigned long lastAlertTime = 0;

// Button debounce
int lastButtonReading = HIGH;
int stableButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

void triggerAlert(String reason) {

  if (alertActive) return;

  if (hasAlerted &&
      millis() - lastAlertTime < cooldown) {
    return;
  }

  alertActive = true;
  hasAlerted = true;

  alertStartTime = millis();
  lastAlertTime = millis();

  Serial.println();
  Serial.println("***********************");
  Serial.println(" ACCIDENT DETECTED!");
  Serial.println("***********************");

  Serial.print("Reason: ");
  Serial.println(reason);

  Serial.println("Sending Emergency SMS...");

  Serial.println("To: Registered Emergency Number");
  Serial.println("VEHICLE ACCIDENT ALERT!");

  Serial.print("Latitude: ");
  Serial.println(latitude, 4);

  Serial.print("Longitude: ");
  Serial.println(longitude, 4);

  Serial.print("Google Maps: https://maps.google.com/?q=");
  Serial.print(latitude, 4);
  Serial.print(",");
  Serial.println(longitude, 4);

  Serial.println("SMS SIMULATION COMPLETED");
  Serial.println();
}

void setup() {

  Serial.begin(9600);
  Wire.begin();

  pinMode(redLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  pinMode(buzzer, OUTPUT);

  pinMode(buttonPin, INPUT_PULLUP);

  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("Accident Alert");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");

  Serial.println("Accident Alert System");
  Serial.println("Initializing...");

  if (!mpu.begin()) {
    Serial.println("MPU6050 NOT FOUND!");
    lcd.clear();
    lcd.print("MPU6050 Error!");
    while (1);
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  delay(1500);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Vehicle Normal");
  lcd.setCursor(0, 1);
  lcd.print("System Ready");

  digitalWrite(greenLED, HIGH);
  digitalWrite(redLED, LOW);
  noTone(buzzer);

  Serial.println("System Ready!");
}

void loop() {

  // Read MPU6050 sensor
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // Calculate acceleration magnitude
  float acceleration = sqrt(
    a.acceleration.x * a.acceleration.x +
    a.acceleration.y * a.acceleration.y +
    a.acceleration.z * a.acceleration.z
  );

  // Automatic accident detection
  if (acceleration > accidentThreshold) {
    triggerAlert("Sudden Impact Detected");
  }

  // Read push button
  int reading = digitalRead(buttonPin);

  if (reading != lastButtonReading) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {

    if (reading != stableButtonState) {
      stableButtonState = reading;

      // Button pressed = manual emergency alert
      if (stableButtonState == LOW) {
        triggerAlert("Manual Emergency Button");
      }
    }
  }

  lastButtonReading = reading;

  // Accident alert condition
  if (alertActive &&
      millis() - alertStartTime < alertDuration) {

    digitalWrite(redLED, HIGH);
    digitalWrite(greenLED, LOW);

    tone(buzzer, 2000);

    lcd.setCursor(0, 0);
    lcd.print("ACCIDENT ALERT  ");
    lcd.setCursor(0, 1);
    lcd.print("Emergency Sent  ");

  } else {

    // Return to normal condition
    alertActive = false;

    digitalWrite(redLED, LOW);
    digitalWrite(greenLED, HIGH);

    noTone(buzzer);

    lcd.setCursor(0, 0);
    lcd.print("Vehicle Normal  ");
    lcd.setCursor(0, 1);
    lcd.print("System Active   ");
  }

  delay(50);
}