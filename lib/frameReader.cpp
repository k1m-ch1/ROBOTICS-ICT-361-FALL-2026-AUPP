#include "frameReader.h"
#include "cobs.h"
#include "frameDecoder.h"
#include "logging.h"
#include "packets.h"
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

      // so this is the set speed
      if (decodedFrame.size != sizeof(SetSpeed)) {
        break;
      }
      SetSpeed *setSpeedCommand = (SetSpeed *)decodedFrame.framePtr;
      frameReaderLogMessage.timestamp = millis();
      sprintf(frameReaderLogMessage.text, "linear: %f, angular: %f",
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
      frameReaderLogMessage.timestamp = millis();
      sprintf(frameReaderLogMessage.text, "linear: %f, angular: %f",
              setSpeedCommand->linear, setSpeedCommand->angular);
      xQueueSend(logQueueHandle, &frameReaderLogMessage, 0);

      break;
    }
    }

    free(decodedFrame.framePtr);
  }
}
