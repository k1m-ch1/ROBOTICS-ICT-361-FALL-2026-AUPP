#pragma once
#include <Arduino.h>
#include <atomic>

// every step is 10 degrees
#define SERVO_ANGLE_STEP_SIZE 10.0f

// TODO: update currentAngle variable everytime we perform a write
extern std::atomic<float> currentAngle;

extern TaskHandle_t dabbleManualModeTaskHandle;

void dabbleManualModeInit();

void dabbleManualModeTask(void *args);
