#pragma once

#include "ultrasonic.h"
#include "config.h"
#include "logging.h"
#include <Arduino.h>
#include <stdint.h>
#include <stdio.h>

QueueHandle_t ultrasonicQueueHandle;

float getDistanceCentimeters(uint32_t pulsePeriod) {
  // assume that the speed of sound is 343m/s = 34300cm/s = 34300/1000_000 cm/us
  // = 0.0343cm/us

  // the distance we want to calculate is d, the distance traveled is 2d,
  // traveled in pulsePeriod amount of time assme that speed of sound is
  // constant, then: 2d = v t => d = (v t)/2 = 0.5 * 0.0343 * t
  return 0.5 * 0.0343 * pulsePeriod;
}

void ultrasonicInit() {
  pinMode(ultrasonicPins.echo, INPUT);
  pinMode(ultrasonicPins.trig, OUTPUT);
  digitalWrite(ultrasonicPins.trig, LOW);
  ultrasonicQueueHandle =
      xQueueCreate(ULTRASONIC_QUEUE_SIZE, sizeof(UltrasonicData));
  xTaskCreate(ultrasonicTask, "Dabble Orchestrator Task", 4096, nullptr, 1,
              nullptr);
}

void ultrasonicTask(void *args) {
  LogMessage ultrasonicLogMessage;
  ultrasonicLogMessage.logSource = ULTRASONIC;
  constexpr TickType_t period = pdMS_TO_TICKS(1000 / ULTRASONIC_POLLING_RATE);
  TickType_t lastWakeTime = xTaskGetTickCount();

  uint32_t pulsePeriodMicroseconds;
  UltrasonicData ultrasonicData;
  while (true) {

    digitalWrite(ultrasonicPins.trig, HIGH);
    delayMicroseconds(10);
    digitalWrite(ultrasonicPins.trig, LOW);
    // read a HIGH pulse
    while (!digitalRead(ultrasonicPins.echo)) {
      // do nothing until it goes HIGH
    }
    pulsePeriodMicroseconds = micros();
    while (digitalRead(ultrasonicPins.echo)) {
      // we need to detect timeout
      if (micros() - pulsePeriodMicroseconds > ULTRASONIC_TIMEOUT_US) {
        break;
      }
      // now we're reading the pulse
    }
    pulsePeriodMicroseconds = micros() - pulsePeriodMicroseconds;

    // record the time we got the LOW pulse
    ultrasonicData.timestamp = millis();
    ultrasonicData.pulsePeriod = pulsePeriodMicroseconds;
    // NOTE: if no one consumes in time, new data won't go into the queue, and
    // the queue will be filled with left over data. Perhaps not the
    // best, but, we assume that the consumer gets started quite quickly and
    // starts consuming immediately
    xQueueSend(ultrasonicQueueHandle, &ultrasonicData, 0);

    ultrasonicLogMessage.timestamp = millis();
    sprintf(ultrasonicLogMessage.text, "Got a pulse of %lu microseconds",
            pulsePeriodMicroseconds);
    xQueueSend(logQueueHandle, &ultrasonicLogMessage, 0);
    // assume that we've already initialized the pins
    vTaskDelayUntil(&lastWakeTime, period);
  }
}
