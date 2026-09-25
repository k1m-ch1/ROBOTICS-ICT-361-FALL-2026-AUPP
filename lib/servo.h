#pragma once

#include "config.h"
#include <atomic>
#include <stdint.h>

typedef struct {
  uint32_t freq;
  uint8_t resolution;
  float minAngle;
  float maxAngle;
  uint32_t minPulseMicroseconds;
  uint32_t maxPulseMicroseconds;
} ServoConfig;

// minPulseMicroseconds is the HIGH pulse period that you need to send in order
// to get the servo to move to 0 degrees. This also goes for
// maxPulseMicroseconds, but for 180 degrees.

// let's suppose we have 0.5ms pulse for 0 deg, and 2.5ms for 180 deg, although,
// normally, it's 1ms for 0 deg and 2ms for 180 deg.

const ServoConfig servoConfig = {.freq = 50,
                                 .resolution = 16,
                                 .minAngle = 30.0f,
                                 .maxAngle = 140.0f,
                                 .minPulseMicroseconds = 500,
                                 .maxPulseMicroseconds = 2500};

#define ABSOLUTE_MAX_SERVO_ANGLE 180.0f

float getDutyCycle(uint32_t freq, uint32_t highPulsePeriodMicroseconds);

void servoInit();

void servoWrite(float angle);
