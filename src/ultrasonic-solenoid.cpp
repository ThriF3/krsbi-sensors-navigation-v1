#include <Arduino.h>

const int trigPin = 33;
const int echoPin = 32;
const int mosfetPin = 14;

#define SOUND_SPEED 0.034
const float distanceThreshold = 20.0;

// INIT FUNCTION
void ultrasonicInit() {
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
    pinMode(mosfetPin, OUTPUT);
    digitalWrite(mosfetPin, LOW);
}

// MAIN LOGIC FUNCTION
void ultrasonicUpdate() {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);

    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    long duration = pulseIn(echoPin, HIGH);
    float distanceCm = duration * SOUND_SPEED / 2;

    if (distanceCm <= 0 || distanceCm > 400) {
        Serial.println("Out of range");
        digitalWrite(mosfetPin, LOW);
    } else {
        Serial.print("Jarak: ");
        Serial.println(distanceCm);

        if (distanceCm < distanceThreshold) {
            // digitalWrite(mosfetPin, HIGH);
            Serial.println("MOSFET ON");
        } else {
            // digitalWrite(mosfetPin, LOW);
            Serial.println("MOSFET OFF");
        }
    }
}
