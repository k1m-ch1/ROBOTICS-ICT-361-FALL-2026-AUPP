#pragma once

#include "servo.h"
#include "config.h"
#include "logging.h"
#include <Arduino.h>

void servoInit() {
  // 10 bits resolution is fine?
  ledcAttach(SERVO_PIN, servoConfig.freq, servoConfig.resolution);
}

float getDutyCycle(uint32_t freq, uint32_t highPulsePeriodMicroseconds) {
  // gives a duty cycle given a high pulse period (useful for ESCs and Servos)
  // if we have a frequency in Hz, then every period is 1 000 000 / freq
  // microseconds

  float periodMicroseconds = 1e6 / ((float)freq);

  return (highPulsePeriodMicroseconds / periodMicroseconds);
}

void servoWrite(float angle) {
  LogMessage servoLogMessage;
  servoLogMessage.logSource = SERVO;
  servoLogMessage.timestamp = millis();
  float t = angle / ABSOLUTE_MAX_SERVO_ANGLE;
  float minDutyCycle =
      getDutyCycle(servoConfig.freq, servoConfig.minPulseMicroseconds);
  float maxDutyCycle =
      getDutyCycle(servoConfig.freq, servoConfig.maxPulseMicroseconds);

  float duty = (1 - t) * minDutyCycle + t * maxDutyCycle;
  uint32_t dutyToWrite = (uint32_t)(duty * (1 << servoConfig.resolution));
  ledcWrite(SERVO_PIN, dutyToWrite);
  sprintf(servoLogMessage.text, "angle: %f, duty: %f, dutyToWrite: %lu", angle,
          duty, dutyToWrite);
  xQueueSend(logQueueHandle, &servoLogMessage, 0);
  /*
  sprintf(servoLogMessage.text, "minDutyCycle: %f, maxDutyCycle: %f",
          minDutyCycle, maxDutyCycle);
  xQueueSend(logQueueHandle, &servoLogMessage, 0);
  */
}
