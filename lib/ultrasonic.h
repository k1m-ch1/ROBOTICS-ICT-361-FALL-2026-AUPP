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

extern QueueHandle_t ultrasonicQueueHandle;

// let's say that if the distance is any farther than 10 meters (which takes d =
// vt/2 => t = 2d/v = 2 * (1000)/343  wihch is around 5 seconds, we stop)

#define ULTRASONIC_TIMEOUT_US 5e6

float getDistanceCentimeters(uint32_t pulsePeriod);

void ultrasonicInit();

void ultrasonicTask(void *args);
