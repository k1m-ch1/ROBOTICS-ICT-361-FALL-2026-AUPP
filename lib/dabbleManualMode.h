#pragma once
#include <atomic>

// every step is 10 degrees
#define SERVO_ANGLE_STEP_SIZE 10

// TODO: update currentAngle variable everytime we perform a write
extern std::atomic<float> currentAngle;
