#pragma once
#include "config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <stdint.h>

#define ULTRASONIC_QUEUE_SIZE 4
#define ULTRASONIC_POLLING_RATE 10

typedef struct {
  uint32_t pulsePeriod;
  uint32_t timestamp;
} UltrasonicData;

extern QueueHandle_t ultrasonicQueue;

float getDistanceCentimeters(uint32_t pulsePeriod);

void ultrasonicInit();

void ultrasonicTask(void *args);
