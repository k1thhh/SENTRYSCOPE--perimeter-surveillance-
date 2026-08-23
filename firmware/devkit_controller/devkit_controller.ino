// ESP32 DevKit Controller Code

#include <Arduino.h>

// Pins
#define IR_PROX_PIN 33
#define IR_BEAM_PIN 26
#define US_TRIG_PIN 5
#define US_ECHO_PIN 18
#define LED_PIN 25
#define SERVO_PIN 14
#define CAM_TRIGGER_PIN 23

//  FIXED LOGIC LEVEL
#define BEAM_INTACT_LEVEL HIGH
#define PROX_ACTIVE_LEVEL LOW  // ← FIXED

#define US_DETECT_CM 10.0f

// Servo PWM
#define SERVO_FREQ_HZ 50
#define SERVO_RES_BITS 16

#define SERVO_MIN_US 500
#define SERVO_MAX_US 2500

int servoHome = 120;
int servoProxAngle = 150;
int servoBeamAngle = 90;
int servoUSAngle = 150;

#define SERVO_SETTLE_MS 350
#define CAPTURE_HOLD_MS 1500
#define RETURN_SETTLE_MS 350
#define COOLDOWN_MS 600

enum SystemState { READY, TARGET_SETTLE, CAPTURE_HOLD, RETURN_SETTLE, COOLDOWN };
SystemState sysState = READY;

unsigned long stateStart = 0;
int targetAngle = 90;

// ---------------- Ultrasonic ----------------
float readUltrasonicCM() {
  digitalWrite(US_TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(US_TRIG_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(US_TRIG_PIN, LOW);

  unsigned long duration = pulseIn(US_ECHO_PIN, HIGH, 25000UL);
  if (duration == 0) return -1.0;

  return (duration * 0.0343) / 2.0;
}

// ---------------- Servo ----------------
void setServoAngle(int angle) {
  angle = constrain(angle, 0, 180);

  int us = SERVO_MIN_US +
           ((SERVO_MAX_US - SERVO_MIN_US) * angle) / 180;

  uint32_t duty = ((1 << SERVO_RES_BITS) - 1) * us / 20000;

  // 🔥 FIXED FOR ESP32 v3
  ledcWrite(SERVO_PIN, duty);
}

// ---------------- Camera Trigger ----------------
void triggerCamera() {
  Serial.println("📸 Triggering Camera");

  digitalWrite(CAM_TRIGGER_PIN, LOW);
  delay(CAPTURE_HOLD_MS);
  digitalWrite(CAM_TRIGGER_PIN, HIGH);
}

// ---------------- Setup ----------------
void setup() {
  Serial.begin(115200);
  Serial.println("\nIntrusion Sentinel - IR Prox + Beam + Ultrasonic");

  pinMode(IR_PROX_PIN, INPUT);
  pinMode(IR_BEAM_PIN, INPUT);
  pinMode(US_TRIG_PIN, OUTPUT);
  pinMode(US_ECHO_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);

  pinMode(CAM_TRIGGER_PIN, OUTPUT_OPEN_DRAIN);
  digitalWrite(CAM_TRIGGER_PIN, HIGH);

  // 🔥 FIXED PWM INIT
  ledcAttach(SERVO_PIN, SERVO_FREQ_HZ, SERVO_RES_BITS);

  setServoAngle(servoHome);
  digitalWrite(LED_PIN, LOW);

  Serial.println("System ready!");
}

// ---------------- Main Loop ----------------
void loop() {

  bool proxDetect = (digitalRead(IR_PROX_PIN) == PROX_ACTIVE_LEVEL);
  bool beamBroken = (digitalRead(IR_BEAM_PIN) != BEAM_INTACT_LEVEL);

  float dist1 = readUltrasonicCM();
  delay(8);
  float dist2 = readUltrasonicCM();

  float distance = (dist1 < 0 && dist2 < 0) ? -1.0 : min(dist1, dist2);
  bool usDetect = (distance > 0 && distance <= US_DETECT_CM);

  bool anyDetect = proxDetect || beamBroken || usDetect;

  digitalWrite(LED_PIN, anyDetect ? HIGH : LOW);

  switch (sysState) {

    case READY:
      if (anyDetect) {

        if (beamBroken)
          targetAngle = servoBeamAngle;
        else if (proxDetect)
          targetAngle = servoProxAngle;
        else if (usDetect)
          targetAngle = servoUSAngle;

        setServoAngle(targetAngle);

        sysState = TARGET_SETTLE;
        stateStart = millis();

        Serial.printf("Detection -> Servo %d° | prox=%d beam=%d us=%d\n",
                      targetAngle, proxDetect, beamBroken, usDetect);
      }
      break;

    case TARGET_SETTLE:
      if (millis() - stateStart >= SERVO_SETTLE_MS) {

        triggerCamera();
        setServoAngle(servoHome);

        sysState = RETURN_SETTLE;
        stateStart = millis();

        Serial.println("Capture done. Returning home...");
      }
      break;

    case RETURN_SETTLE:
      if (millis() - stateStart >= RETURN_SETTLE_MS) {
        sysState = COOLDOWN;
        stateStart = millis();
      }
      break;

    case COOLDOWN:
      if (millis() - stateStart >= COOLDOWN_MS) {
        sysState = READY;
        Serial.println("System ready for next detection.");
      }
      break;
  }

  delay(5);
}