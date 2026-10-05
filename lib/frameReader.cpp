#include "frameReader.h"
#include "cobs.h"
#include "frameDecoder.h"
#include "logging.h"
#include <stdio.h>

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
    sprintf(frameReaderLogMessage.text, "size %d, packetID: %X",
            decodedFrame.size, decodedFrame.framePtr[0]);
    xQueueSend(logQueueHandle, &frameReaderLogMessage, 0);
  }
}
