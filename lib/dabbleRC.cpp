#pragma once

#include "dabbleOrchestrator.h"
#include "logging.h"

#define CUSTOM_SETTINGS
#define INCLUDE_GAMEPAD_MODULE

#include <Arduino.h>
#include <DabbleESP32.h>

#include "dabbleRC.h"

DabbleRCButtonState prevDabbleRCButtonState;
DabbleRCButtonState dabbleRCButtonState;

SemaphoreHandle_t dabbleRCButtonStateMutex;

void dabbleRCInit() {
  Dabble.begin(BT_NAME);
  // zero out the button state when initializing, not even going to protect teh
  // mutex yet because we haven't initialized it, since it's notiication based,
  // no one is going to access it yet
  dabbleRCButtonStateMutex = xSemaphoreCreateMutex();
  dabbleRCButtonState = {};
  prevDabbleRCButtonState = {};
  xTaskCreate(dabbleRCTask, "Dabble RC Task", 4096, nullptr, 1, nullptr);
}

void dabbleRCTask(void *args) {
  LogMessage dabbleLogMessage;
  dabbleLogMessage.timestamp = millis();
  dabbleLogMessage.logSource = DABBLE_RC;

  constexpr TickType_t period = pdMS_TO_TICKS(1000 / DABBLE_RC_POLLING_RATE);
  TickType_t lastWakeTime;
  while (true) {
    lastWakeTime = xTaskGetTickCount();

    /*
    dabbleLogMessage.timestamp = millis();
    sprintf(dabbleLogMessage.text, "hello from dabble task");
    xQueueSend(logQueueHandle, &dabbleLogMessage, 0);
    */

    // let's now start the polling
    Dabble.processInput();

    // here comes the repetitive code...

    xSemaphoreTake(dabbleRCButtonStateMutex,
                   portMAX_DELAY); // wait indefinitely
    prevDabbleRCButtonState = dabbleRCButtonState;
    dabbleRCButtonState.up = GamePad.isUpPressed();
    dabbleRCButtonState.down = GamePad.isDownPressed();
    dabbleRCButtonState.left = GamePad.isLeftPressed();
    dabbleRCButtonState.right = GamePad.isRightPressed();
    dabbleRCButtonState.square = GamePad.isSquarePressed();
    dabbleRCButtonState.triangle = GamePad.isTrianglePressed();
    dabbleRCButtonState.circle = GamePad.isCirclePressed();
    dabbleRCButtonState.cross = GamePad.isCrossPressed();
    dabbleRCButtonState.start = GamePad.isStartPressed();
    dabbleRCButtonState.select = GamePad.isSelectPressed();
    xSemaphoreGive(dabbleRCButtonStateMutex);

    xTaskNotifyGive(dabbleOrchestratorTaskHandle);

    vTaskDelayUntil(&lastWakeTime, period);
  }
}
