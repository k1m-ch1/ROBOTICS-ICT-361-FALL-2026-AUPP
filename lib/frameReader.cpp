#include "frameReader.h"
#include "cobs.h"
#include "frameDecoder.h"
#include "logging.h"
#include "mixer.h"
#include "packets.h"
#include "servo.h"
#include <Arduino.h>
#include <stdio.h>
#include <stdlib.h>

void frameReaderInit() {
  xTaskCreate(frameReaderTask, "Frame Reader Task", 4096, nullptr, 1, nullptr);
}

void frameReaderTask(void *args) {
  LogMessage frameReaderLogMessage;
  frameReaderLogMessage.logSource = FRAME_READER;
  Frame decodedFrame;
  while (true) {
    // we receive from the queue
    xQueueReceive(decodedFrameQueueHandle, &decodedFrame, portMAX_DELAY);
    frameReaderLogMessage.timestamp = millis();
    sprintf(frameReaderLogMessage.text, "size %d, packetID: %X",
            decodedFrame.size, decodedFrame.framePtr[0]);
    xQueueSend(logQueueHandle, &frameReaderLogMessage, 0);

    switch (decodedFrame.framePtr[0]) {
    case 0x01: {
      if (decodedFrame.size != sizeof(SetSpeed)) {
        // if this runs, then it means that we probably got a corrupted packet
        break;
      }
      SetSpeed *setSpeedCommand = (SetSpeed *)decodedFrame.framePtr;

      xSemaphoreTake(speedMutex, portMAX_DELAY);
      speed.linear = setSpeedCommand->linear;
      speed.angular = setSpeedCommand->angular;
      xSemaphoreGive(speedMutex);
      xTaskNotifyGive(mixerTaskHandle);

      frameReaderLogMessage.timestamp = millis();
      sprintf(frameReaderLogMessage.text, "[SetSpeed] linear: %f, angular: %f",
              setSpeedCommand->linear, setSpeedCommand->angular);
      xQueueSend(logQueueHandle, &frameReaderLogMessage, 0);
      break;
    }
    case 0x02: {
      // this is the set speed limit struct
      if (decodedFrame.size != sizeof(SetSpeedLimit)) {
        break;
      }
      SetSpeedLimit *setSpeedCommand = (SetSpeedLimit *)decodedFrame.framePtr;
      xSemaphoreTake(speedLimitMutex, portMAX_DELAY);
      speedLimit.linear = setSpeedCommand->linear;
      speedLimit.angular = setSpeedCommand->angular;
      xSemaphoreGive(speedLimitMutex);
      frameReaderLogMessage.timestamp = millis();
      sprintf(frameReaderLogMessage.text,
              "[SetSpeedLimit] linear: %f, angular: %f",
              setSpeedCommand->linear, setSpeedCommand->angular);
      xQueueSend(logQueueHandle, &frameReaderLogMessage, 0);
      break;
    }
    case 0x03: {
      if (decodedFrame.size != sizeof(SetServoAngle)) {
        break;
      }
      SetServoAngle *setServoAngle = (SetServoAngle *)decodedFrame.framePtr;
      servoWrite(setServoAngle->angle);
      frameReaderLogMessage.timestamp = millis();
      sprintf(frameReaderLogMessage.text, "[SetServoAngle] angle: %f",
              setServoAngle->angle);
      xQueueSend(logQueueHandle, &frameReaderLogMessage, 0);
    }
    }
    free(decodedFrame.framePtr);
  }
}
