#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

// the frame decoder sits directly on the serial port

#define DECODED_FRAME_QUEUE_SIZE 8
#define MAX_FRAME_SIZE 1024

extern QueueHandle_t decodedFrameQueueHandle;

void frameDecoderInit();

void frameDecoderTask(void *args);
