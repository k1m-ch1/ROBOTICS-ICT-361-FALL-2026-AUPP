#pragma once

#include "servo.h"
#include "config.h"
#include "logging.h"
#include <Arduino.h>

std::atomic<float> currentAngle;

void servoInit() {
  ledcAttach(servoConfig.freq, servoConfig.resolution, SERVO_PIN);
}

float getDutyCycle(uint32_t freq, uint32_t highPulsePeriodMicroseconds) {
  // gives a duty cycle given a high pulse period (useful for ESCs and Servos)
  // if we have a frequency in Hz, then every period is 1 000 000 / freq
  // microseconds

  float periodMicroseconds = 10e6 / ((float)freq);

  return (highPulsePeriodMicroseconds / periodMicroseconds);
}

void servoWrite(float angle) {
  float t = angle / ABSOLUTE_MAX_SERVO_ANGLE;
  float duty =
      (1 - t) *
          getDutyCycle(servoConfig.freq, servoConfig.minPulseMicroseconds) +
      t * getDutyCycle(servoConfig.freq, servoConfig.maxPulseMicroseconds);
  ledcWrite(SERVO_PIN, duty * (1 << servoConfig.resolution));
}
