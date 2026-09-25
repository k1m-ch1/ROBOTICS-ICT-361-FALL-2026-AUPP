#pragma once

#include "dabbleAutoMode.h"
#include "dabbleOrchestrator.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "logging.h"
#include "mixer.h"
#include "ultrasonic.h"
#include <Arduino.h>

DabbleAutoModeState currentDabbleAutoModeState;

const char *
getDabbleAutoModeStateName(DabbleAutoModeState dabbleAutoModeState) {
  switch (dabbleAutoModeState) {
  case SEARCHING:
    return "SEARCHING";
  case TRAVELLING:
    return "TRAVELLING";
  default:
    return "UNKNOWN";
  }
}

void dabbleAutoModeInit() {
  // pinned to core 1
  // start in searching mode
  currentDabbleAutoModeState = SEARCHING;
  xTaskCreatePinnedToCore(dabbleAutoModeTask, "Dabble Auto Mode Task", 8192,
                          nullptr, 1, nullptr, 1);
}

void dabbleAutoModeTask(void *args) {
  LogMessage dabbleAutoModeLogMessage;
  UltrasonicData ultrasonicData;
  dabbleAutoModeLogMessage.logSource = DABBLE_AUTO_MODE;

  while (true) {
    xQueueReceive(ultrasonicQueueHandle, &ultrasonicData, portMAX_DELAY);

    // we only want to run the state machine when we're in AUTO mode. So first
    // we need to grab the states mutex from the orchestrator

    xSemaphoreTake(currentDabbleRCModeMutex,
                   portMAX_DELAY); // wait indefinitely

    if (currentDabbleRCMode != AUTO) {
      xSemaphoreGive(currentDabbleRCModeMutex);
      continue;
    }
    // now that we know that we're in AUTO mode, we can just continue and give
    // back the mutex
    xSemaphoreGive(currentDabbleRCModeMutex);

    dabbleAutoModeLogMessage.timestamp = millis();

    // first, we need the transition conditions

    float distanceCentimeters =
        getDistanceCentimeters(ultrasonicData.pulsePeriod);

    // if the distance is below MIN_ULTRASONIC_DISTANCE_CM, regard that there
    // is an obstacle in front of the car
    bool obstacleInFront = distanceCentimeters < MIN_ULTRASONIC_DISTANCE_CM;

    // grab the speed mutex from mixer immediately and then notify mixer also

    xSemaphoreTake(speedMutex, portMAX_DELAY); // wait indefinitely

    switch (currentDabbleAutoModeState) {
    case TRAVELLING:
      // if we're in the travelling mode, we move to the SEARCHING state if
      // there is an obstacle in front

      if (obstacleInFront) {
        // since there is an obstacle in the front, we transition to the
        // SEARCHING state, also, cause side effects when transitioning
        speed.linear = searchingSpeed.linear;
        speed.angular = searchingSpeed.angular;
        currentDabbleAutoModeState = SEARCHING;
        sprintf(dabbleAutoModeLogMessage.text, "%f cm. Transitioning %s to %s",
                distanceCentimeters, getDabbleAutoModeStateName(TRAVELLING),
                getDabbleAutoModeStateName(SEARCHING));
        xQueueSend(logQueueHandle, &dabbleAutoModeLogMessage, 0);
      } else {
        speed.linear = travellingSpeed.linear;
        speed.angular = travellingSpeed.angular;
      }
      break;
    case SEARCHING:
      // if we're in the searching state, we transition when there isn't an
      // obstacle in front anymore
      if (!obstacleInFront) {
        // since there are no obstacle in the front, we change back to the
        // travelling state (and cause side-effects when transitioning)
        speed.linear = travellingSpeed.linear;
        speed.angular = travellingSpeed.angular;
        currentDabbleAutoModeState = TRAVELLING;
        sprintf(dabbleAutoModeLogMessage.text, "%f cm. Transitioning %s to %s",
                distanceCentimeters, getDabbleAutoModeStateName(SEARCHING),
                getDabbleAutoModeStateName(TRAVELLING));
        xQueueSend(logQueueHandle, &dabbleAutoModeLogMessage, 0);
      } else {
        speed.linear = searchingSpeed.linear;
        speed.angular = searchingSpeed.angular;
      }
      break;
    }
    xSemaphoreGive(speedMutex);
    xTaskNotifyGive(mixerTaskHandle);
  }
}
