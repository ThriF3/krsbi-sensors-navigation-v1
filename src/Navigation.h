#ifndef NAVIGATION_H
#define NAVIGATION_H

#include <Arduino.h>

class Navigation {
  private:
    static const float WHEEL_ANGLES[3];

    static constexpr float L = 1.0f;

    const int pwmPin[3] = {26, 25, 19};
    const int in1Pin[3] = {33, 12, 18};
    const int in2Pin[3] = {32, 13, 5};

    void setMotor(int idx, float speed);

  public:
    void begin();
    void stopRobot();
    void moveRobot(float vx, float vy, float omega);
};

#endif