#include "frameDecoder.h"
#include "cobs.h"
#include "config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "logging.h"
#include <Arduino.h>

QueueHandle_t decodedFrameQueueHandle;

void frameDecoderInit() {
  // start the serial output if it hasn't been done already
  Serial.begin(SERIAL_BAUD_RATE);

  // create the decoded frame queue
  decodedFrameQueueHandle =
      xQueueCreate(DECODED_FRAME_QUEUE_SIZE, sizeof(Frame));
  xTaskCreatePinnedToCore(frameDecoderTask, "Frame Decoder Task", 4096, nullptr,
                          1, nullptr, 0);
}

void frameDecoderTask(void *args) {
  LogMessage frameDecoderLogMessage;
  Frame rawFrame;
  Frame decodedFrame;
  frameDecoderLogMessage.logSource = FRAME_DECODER;
  uint8_t buffer[MAX_FRAME_SIZE];
  uint32_t occupied = 0;
  uint8_t incomingByte;

  while (true) {
    // want to emulate blocking read
    if (Serial.available() == 0) {
      // wait for one millisecond in order to give other tasks some CPU time
      vTaskDelay(pdMS_TO_TICKS(1));
      continue;
    }
    incomingByte = Serial.read();

    if (incomingByte == 0x00) {
      // if it's a delimiter, then do something about the buffer and send it
      // to the queue
      rawFrame.framePtr = buffer;
      rawFrame.size = occupied;
      decodedFrame = cobsDecode(rawFrame);
      occupied = 0;

      if (decodedFrame.size != 0) {
        // push it to the queue only if it's valid
        // NOTE: whoever receives the frame must free the memory to prevent
        // memory leakage.
        xQueueSend(decodedFrameQueueHandle, &decodedFrame, 0);
      }
      continue;
    }
    // if the buffer overflowed, just let the RTOS throw an exception
    buffer[occupied] = incomingByte;
    occupied++;
  }
}
