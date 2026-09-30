#ifndef COBS_H
#define COBS_H
#include <stddef.h>
#include <stdint.h>

typedef struct {
  uint8_t *framePtr;
  uint32_t size; // i don't think we'll be sending more than 4 GB in one frame
} Frame;

Frame cobsEncode(Frame decodedFrame);
Frame cobsDecode(Frame encodedFrame);
#endif
