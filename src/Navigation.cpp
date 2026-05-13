#include "Navigation.h"

const float Navigation::WHEEL_ANGLES[3] = {
  0.0f,
  2.0943951f,
  4.1887902f
};

void Navigation::begin() {
  for (int i = 0; i < 3; i++) {
    pinMode(pwmPin[i], OUTPUT);
    pinMode(in1Pin[i], OUTPUT);
    pinMode(in2Pin[i], OUTPUT);
  }

  stopRobot();
}

void Navigation::setMotor(int idx, float speed) {
  speed = constrain(speed, -1.0, 1.0);

  int pwmValue = (int)(fabs(speed) * 255.0);

  if (speed >= 0) {
    digitalWrite(in1Pin[idx], HIGH);
    digitalWrite(in2Pin[idx], LOW);
  } else {
    digitalWrite(in1Pin[idx], LOW);
    digitalWrite(in2Pin[idx], HIGH);
  }

  analogWrite(pwmPin[idx], pwmValue);
}

void Navigation::stopRobot() {
  for (int i = 0; i < 3; i++) {
    analogWrite(pwmPin[i], 0);
    digitalWrite(in1Pin[i], LOW);
    digitalWrite(in2Pin[i], LOW);
  }
}

void Navigation::moveRobot(float vx, float vy, float omega) {
  float wheel[3];

  // Inverse kinematics / movement matrix
  for (int i = 0; i < 3; i++) {
    wheel[i] = (-sin(WHEEL_ANGLES[i]) * vx) +
               ( cos(WHEEL_ANGLES[i]) * vy) +
               ( L * omega);
  }

  // Normalize wheel speeds
  float maxMag = 0.0;

  for (int i = 0; i < 3; i++) {
    if (fabs(wheel[i]) > maxMag) {
      maxMag = fabs(wheel[i]);
    }
  }

  if (maxMag > 1.0) {
    for (int i = 0; i < 3; i++) {
      wheel[i] /= maxMag;
    }
  }

  for (int i = 0; i < 3; i++) {
    setMotor(i, wheel[i]);
  }
}