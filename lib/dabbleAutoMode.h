#pragma once

#include "mixer.h"

// if the distance is less than MIN_ULTRASONIC_DISTANCE_CM, then we turn left
#define MIN_ULTRASONIC_DISTANCE_CM 60.0f

// so our state machine has 2 states:
// - searching
// - traveling

// and we have one transition:

const Speed travellingSpeed = {.linear = 1.0f, .angular = 0.0f};

// when searching, we just turn left, which is .angular == 1.0f
const Speed searchingSpeed = {.linear = 0.0f, .angular = 1.0f};

typedef enum { TRAVELLING, SEARCHING } DabbleAutoModeState;

const char *getDabbleAutoModeStateName(DabbleAutoModeState dabbleAutoModeState);

void dabbleAutoModeInit();

void dabbleAutoModeTask(void *args);
