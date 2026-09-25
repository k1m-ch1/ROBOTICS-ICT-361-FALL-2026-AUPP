#pragma once

#include "dabbleManualMode.h"
#include "dabbleOrchestrator.h"
#include "dabbleRC.h"
#include "logging.h"
#include "mixer.h"
#include "servo.h"
#include "utils.h"
#include <Arduino.h>
#include <atomic>

TaskHandle_t dabbleManualModeTaskHandle;
std::atomic<float> currentAngle;

void dabbleManualModeInit() {
  // i'm going to use a .store as opposed to regular functions with syntactic
  // sugar to distinguish it with regular non-atomic primitives

  currentAngle.store(servoConfig.minAngle);
  servoWrite(currentAngle.load());
  xTaskCreate(dabbleManualModeTask, "Dabble Manual Mode Task", 4096, nullptr, 1,
              &dabbleManualModeTaskHandle);
}

void dabbleManualModeTask(void *args) {

  LogMessage dabbleManualModeLogMessage;
  dabbleManualModeLogMessage.logSource = DABBLE_MANUAL_MODE;

  while (true) {
    dabbleManualModeLogMessage.timestamp = millis();
    // blocking notify
    xTaskNotifyWait(0,            // bits to clear on entry
                    0,            // bits to clear on exit
                    nullptr,      // where to put notification value
                    portMAX_DELAY // wait forever
    );

    // check that we're in manual mode

    // first take the mutex

    xSemaphoreTake(currentDabbleRCModeMutex,
                   portMAX_DELAY); // wait indefinitely

    if (currentDabbleRCMode != MANUAL) {
      xSemaphoreGive(currentDabbleRCModeMutex);
      continue;
    }
    // since  we already know that we're in manual mode, we don't need this
    // mutex anymore
    xSemaphoreGive(currentDabbleRCModeMutex);

    // now, we're guaranteed that we're in manual mode. If we're in manual mode,
    // we're going to first take the button state mutex.

    xSemaphoreTake(dabbleRCButtonStateMutex,
                   portMAX_DELAY); // wait indefinitely

    // consume only, also, right now, we're causing the robot to move, as such,
    // we must take the speed mutex, and then notify the mixer

    xSemaphoreTake(speedMutex, portMAX_DELAY); // wait indefinitely

    // if none of these buttons are pressed, then set the speed back to 0
    speed.linear = 0.0f;
    speed.angular = 0.0f;
    if (dabbleRCButtonState.left) {
      // to turn left, set the linear speed to 0 and angular speed to 1
      speed.angular = 1.0f;
    }
    if (dabbleRCButtonState.right) {
      speed.angular = -1.0f;
    }
    if (dabbleRCButtonState.up) {
      speed.linear = 1.0f;
    }
    if (dabbleRCButtonState.down) {
      speed.linear = -1.0f;
    }
    sprintf(dabbleManualModeLogMessage.text, "linear: %f, angular: %f",
            speed.linear, speed.angular);
    xQueueSend(logQueueHandle, &dabbleManualModeLogMessage, 0);

    // we do this because, if we were to press left and up at the same time for
    // instance, we still want it to do some stuff unlock the mutex we want to
    // mix it together

    // give the notification to the mixer task for it to handle
    xSemaphoreGive(speedMutex);
    xTaskNotifyGive(mixerTaskHandle);

    // now we're checking whether we want the servo to move and stuff

    if (detectEdge(prevDabbleRCButtonState.square, dabbleRCButtonState.square,
                   EDGE_RISING)) {
      // if it's square, increase angle by 10

      currentAngle += SERVO_ANGLE_STEP_SIZE;
      sprintf(dabbleManualModeLogMessage.text, "SQUARE button registered");
      xQueueSend(logQueueHandle, &dabbleManualModeLogMessage, 0);
    }

    if (detectEdge(prevDabbleRCButtonState.circle, dabbleRCButtonState.circle,
                   EDGE_RISING)) {
      // if it's circle, decrease angle by 10
      currentAngle -= SERVO_ANGLE_STEP_SIZE;
      sprintf(dabbleManualModeLogMessage.text, "CIRCLE button registered");
      xQueueSend(logQueueHandle, &dabbleManualModeLogMessage, 0);
    }

    if (detectEdge(prevDabbleRCButtonState.triangle,
                   dabbleRCButtonState.triangle, EDGE_RISING)) {
      // if it's a triangle, move to servoConfig.maxAngle = 140.0f
      currentAngle.store(servoConfig.maxAngle);
      sprintf(dabbleManualModeLogMessage.text, "TRIANGLE button registered");
      xQueueSend(logQueueHandle, &dabbleManualModeLogMessage, 0);
    }

    if (detectEdge(prevDabbleRCButtonState.cross, dabbleRCButtonState.cross,
                   EDGE_RISING)) {
      // if it's cross, move to servoConfig.minAngle = 30.0f
      currentAngle.store(servoConfig.minAngle);
      sprintf(dabbleManualModeLogMessage.text, "CROSS button registered");
      xQueueSend(logQueueHandle, &dabbleManualModeLogMessage, 0);
    }

    xSemaphoreGive(dabbleRCButtonStateMutex);

    // at the end of it, we load it (this isn't atomic, but there aren't weird
    // cases where one overwrite the other) we clamp it
    currentAngle.store(max(servoConfig.minAngle,
                           min(servoConfig.maxAngle, currentAngle.load())));
    // servoWrite(currentAngle.load());
  }
}
