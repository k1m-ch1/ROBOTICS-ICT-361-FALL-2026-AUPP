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
    if (detectEdge(prevDabbleRCButtonState.start, dabbleRCButtonState.start,
                   EDGE_RISING)) {
      sprintf(dabbleOrchestratorLogMessage.text, "START button pressed");
      xQueueSend(logQueueHandle, &dabbleOrchestratorLogMessage, 0);
    }
    if (detectEdge(prevDabbleRCButtonState.select, dabbleRCButtonState.select,
                   EDGE_RISING)) {
      sprintf(dabbleOrchestratorLogMessage.text, "SELECT button pressed");
      xQueueSend(logQueueHandle, &dabbleOrchestratorLogMessage, 0);
    }
    xSemaphoreGive(dabbleRCButtonStateMutex);
  }
}
