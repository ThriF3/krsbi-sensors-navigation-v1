#include <Arduino.h>
#include "Navigation.h"
#include "MQTTController.h"


// const char* WIFI_SSID = "HOTSPOT_ITENAS";
// const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
// const char* MQTT_SERVER = "efee5f5fd4384475aca6ad0900739372.s1.eu.hivemq.cloud";   // change to your broker IP or domain
// const uint16_t MQTT_PORT = 1883;
// const char* MQTT_TOPIC = "Navigation";

Navigation robot;
// MQTTController mqtt(WIFI_SSID, WIFI_PASSWORD, MQTT_SERVER, MQTT_PORT, MQTT_TOPIC, robot);

// -------------------- Ultrasonic + MOSFET pins --------------------
const int trigPin   = 33;
const int echoPin   = 32;
const int mosfetPin = 14;

#define SOUND_SPEED 0.034f
const float distanceThreshold = 20.0f;

// -------------------- Test timing --------------------
const unsigned long motionDurationMs = 2000;
const unsigned long stopDurationMs   = 500;
unsigned long lastStateChangeMs = 0;

// -------------------- Test sequence --------------------
enum TestStep {
  FORWARD_STEP = 0,
  BACKWARD_STEP,
  LEFT_STEP,
  RIGHT_STEP,
  CW_STEP,
  CCW_STEP,
  TEST_COUNT
};

TestStep currentStep = FORWARD_STEP;

// -------------------- Helper: step name --------------------
const char* stepName(TestStep step) {
  switch (step) {
    case FORWARD_STEP:  return "forward";
    case BACKWARD_STEP:  return "backward";
    case LEFT_STEP:     return "left";
    case RIGHT_STEP:    return "right";
    case CW_STEP:       return "clockwise";
    case CCW_STEP:      return "counter-clockwise";
    default:            return "unknown";
  }
}

// -------------------- Helper: read ultrasonic distance --------------------
float readUltrasonicDistanceCm() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // 30 ms timeout ≈ 5 meters max distance, prevents blocking forever
  unsigned long duration = pulseIn(echoPin, HIGH, 30000UL);

  if (duration == 0) {
    return -1.0f; // no echo / timeout
  }

  return (duration * SOUND_SPEED) / 2.0f;
}

// -------------------- Helper: apply MOSFET based on distance --------------------
void updateMosfetFromDistance(float distanceCm) {
  bool mosfetOn = (distanceCm > 0.0f && distanceCm <= distanceThreshold);
  digitalWrite(mosfetPin, mosfetOn ? HIGH : LOW);

  Serial.print("MOSFET: ");
  Serial.println(mosfetOn ? "ON" : "OFF");
}

// -------------------- Helper: print ultrasonic info --------------------
void printUltrasonic(float distanceCm) {
  Serial.print("Ultrasonic: ");

  if (distanceCm < 0.0f) {
    Serial.println("Out of range / timeout");
  } else {
    Serial.print(distanceCm);
    Serial.println(" cm");
  }
}

// -------------------- Helper: move robot by test step --------------------
void runStep(TestStep step) {
  switch (step) {
    case FORWARD_STEP:
      robot.moveRobot(1.0f, 0.0f, 0.0f);
      break;

    case BACKWARD_STEP:
      robot.moveRobot(-1.0f, 0.0f, 0.0f);
      break;

    case LEFT_STEP:
      robot.moveRobot(0.0f, 1.0f, 0.0f);
      break;

    case RIGHT_STEP:
      robot.moveRobot(0.0f, -1.0f, 0.0f);
      break;

    case CW_STEP:
      robot.moveRobot(0.0f, 0.0f, 1.0f);
      break;

    case CCW_STEP:
      robot.moveRobot(0.0f, 0.0f, -1.0f);
      break;

    default:
      robot.stopRobot();
      break;
  }
}

// -------------------- Arduino setup --------------------
void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(mosfetPin, OUTPUT);
  digitalWrite(mosfetPin, LOW);

  robot.begin();
  robot.stopRobot();

  // mqtt.begin();

  lastStateChangeMs = millis();

  Serial.println("=== Navigation + Ultrasonic + MOSFET Test Start ===");
}

// -------------------- Arduino loop --------------------
void loop() {
  unsigned long now = millis();

  // Run current motion step
  runStep(currentStep);

  // Read ultrasonic sensor every loop
  // float distanceCm = readUltrasonicDistanceCm();

  // Serial output for each instruction/state
  Serial.print("Navigation: ");
  Serial.println(stepName(currentStep));

  // printUltrasonic(distanceCm);
  // updateMosfetFromDistance(distanceCm);
  Serial.println("--------------------------------");

  digitalWrite(mosfetPin, HIGH);

  // Change to the next motion after a fixed duration
  if (now - lastStateChangeMs >= motionDurationMs) {
    robot.stopRobot();
    delay(stopDurationMs);

    currentStep = static_cast<TestStep>((currentStep + 1) % TEST_COUNT);
    lastStateChangeMs = millis();
  }
  // mqtt.loop();
}