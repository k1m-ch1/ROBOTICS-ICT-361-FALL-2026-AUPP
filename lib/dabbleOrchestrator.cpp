#pragma once

/*
extern DabbleRCMode currentDabbleRCMode;
extern SemaphoreHandle_t currentDabbleRCModeMutex;
*/

#include "dabbleOrchestrator.h"
#include "dabbleRC.h"
#include "logging.h"
#include "utils.h"
#include <Arduino.h>

DabbleRCMode currentDabbleRCMode;
SemaphoreHandle_t currentDabbleRCModeMutex;

TaskHandle_t dabbleOrchestratorTaskHandle;

const char *getDabbleRCModeName(DabbleRCMode dabbleRCMode) {
  switch (dabbleRCMode) {
  case IDLE:
    return "IDLE";
  case MANUAL:
    return "MANUAL";
  case AUTO:
    return "AUTO";
  default:
    return "UNKNOWN";
  }
}

void dabbleOrchestratorInit() {
  // first we need to initialize the modes and create the mutex
  currentDabbleRCModeMutex = xSemaphoreCreateMutex();
  currentDabbleRCMode = IDLE;
  xTaskCreate(dabbleOrchestratorTask, "Dabble Orchestrator Task", 4096, nullptr,
              1, &dabbleOrchestratorTaskHandle);
}

void dabbleOrchestratorTask(void *args) {
  // so, essentially, I need this guy to wait until it gets notified by dabbleRC

  LogMessage dabbleOrchestratorLogMessage;
  dabbleOrchestratorLogMessage.logSource = DABBLE_ORCHESTRATOR;
  while (true) {
    dabbleOrchestratorLogMessage.timestamp = millis();
    xTaskNotifyWait(0, 0, nullptr, portMAX_DELAY);

    xSemaphoreTake(dabbleRCButtonStateMutex,
                   portMAX_DELAY); // wait indefinitely
    // consume only
    if (detectEdge(prevDabbleRCButtonState.select, dabbleRCButtonState.select,
                   EDGE_RISING)) {
      xSemaphoreTake(currentDabbleRCModeMutex,
                     portMAX_DELAY); // wait indefinitely
      currentDabbleRCMode = MANUAL;
      xSemaphoreGive(currentDabbleRCModeMutex);

      sprintf(dabbleOrchestratorLogMessage.text,
              "SELECT button pressed. currentDabbleRCMode set to %s",
              getDabbleRCModeName(currentDabbleRCMode));

      xQueueSend(logQueueHandle, &dabbleOrchestratorLogMessage, 0);
    }

    if (detectEdge(prevDabbleRCButtonState.start, dabbleRCButtonState.start,
                   EDGE_RISING)) {
      xSemaphoreTake(currentDabbleRCModeMutex,
                     portMAX_DELAY); // wait indefinitely
      currentDabbleRCMode = AUTO;
      sprintf(dabbleOrchestratorLogMessage.text,
              "START button pressed. currentDabbleRCMode set to %s",
              getDabbleRCModeName(currentDabbleRCMode));
      xSemaphoreGive(currentDabbleRCModeMutex);
      xQueueSend(logQueueHandle, &dabbleOrchestratorLogMessage, 0);
    }
    xSemaphoreGive(dabbleRCButtonStateMutex);
  }
}
