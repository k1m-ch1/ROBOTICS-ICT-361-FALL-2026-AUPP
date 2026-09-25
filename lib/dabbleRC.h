#pragma once

#include "dabbleOrchestrator.h"

// polling period in Hz (because Dabble uses a polling based API)
#define DABBLE_RC_POLLING_RATE 60.0f

#define BT_NAME "k1mch1 ESP32 robot"

typedef struct {
  bool up;
  bool down;
  bool left;
  bool right;
  bool square;
  bool triangle;
  bool circle;
  bool cross;
  bool start;
  bool select;
} DabbleRCButtonState;

extern DabbleRCButtonState prevDabbleRCButtonState;
extern DabbleRCButtonState dabbleRCButtonState;

// here's a weird design decision. Because the previous button state is going to
// be used to detect whether we have a rising edge, falling edge, etc, we're
// actually going to wrap the previous button state with the same mutex
extern SemaphoreHandle_t dabbleRCButtonStateMutex;

void dabbleRCInit();

void dabbleRCTask(void *args);
