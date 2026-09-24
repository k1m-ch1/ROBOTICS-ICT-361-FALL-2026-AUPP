#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

typedef enum { IDLE, MANUAL, AUTO, MODES_AMOUNT } DabbleRCMode;

extern DabbleRCMode currentDabbleRCMode;
extern SemaphoreHandle_t currentDabbleRCModeMutex;

extern TaskHandle_t dabbleOrchestratorTaskHandle;

void dabbleOrchestratorInit();

void dabbleOrchestratorTask(void *args);
