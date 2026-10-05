#include "cobs.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

Frame cobsEncode(Frame decodedFrame) {
  // we'll actually malloc a frame and then put it in the frame and send it back
  // out
  Frame encodedFrame = {};
  encodedFrame.framePtr = (uint8_t *)calloc(
      decodedFrame.size + (decodedFrame.size / 254) + 1, sizeof(uint8_t));
  // we'll now loop through the decodedFrame
  uint32_t offset = 1;
  uint32_t traversedBytes = 0;
  for (uint32_t i = 0; i < decodedFrame.size; i++) {
    traversedBytes++;
    if (traversedBytes == 0xFF) {
      encodedFrame.framePtr[i + offset - traversedBytes] = traversedBytes;
      offset++;
      traversedBytes = 1;
    }

    if (decodedFrame.framePtr[i] == 0x00) {
      encodedFrame.framePtr[i + offset - traversedBytes] = traversedBytes;
      traversedBytes = 0;
      continue;
    }

    encodedFrame.framePtr[i + offset] = decodedFrame.framePtr[i];
  }

  // wrap it up when we reach the end, essentially, we can assume that there
  // is an imaginary 0x00 at the end of the decodedFrame
  traversedBytes++;
  encodedFrame.framePtr[decodedFrame.size + offset - traversedBytes] =
      traversedBytes;

  // at the end, the size should just be decodedFrame.size + offset
  encodedFrame.size = decodedFrame.size + offset;
  return encodedFrame;
}

Frame cobsDecode(Frame encodedFrame) {
  // if the decoded frame can't be decoded, I suppose we can return a nullptr to
  // say that we got an error

  // we can guarantee that the encodedFrame.size - 1 >= decodedFrame.size

  Frame decodedFrame = {};
  decodedFrame.framePtr =
      (uint8_t *)calloc(encodedFrame.size - 1, sizeof(uint8_t));

  // first thing first, we take the traversal number and store it
  uint32_t traversals = encodedFrame.framePtr[0];
  bool isNextDestinationAPtr = traversals == 0xFF;
  uint32_t decodedFramePtr = 0;
  for (uint32_t i = 1; i < encodedFrame.size; i++) {
    // everytime we loop through this, we've essentially traversed by one
    traversals--;
    if (traversals == 0) {
      // if we have no traversals left, then we've either reached a 0x00 byte,
      // or another pointer
      if (!isNextDestinationAPtr) {
        decodedFrame.framePtr[decodedFramePtr] = 0x00;
        decodedFramePtr++;
      }
      traversals = encodedFrame.framePtr[i];
      isNextDestinationAPtr = traversals == 0xFF;
      continue;
    }
    decodedFrame.framePtr[decodedFramePtr] = encodedFrame.framePtr[i];
    decodedFramePtr++;
  }
  // decoded frame ptr is pointer to the next index to insert, so at the end,
  // that index will be the length
  decodedFrame.size = decodedFramePtr;
  // if we have one traversal left (because we need just one more traversal to
  // reach the delimiter), then that's good, otherwise, something went wrong
  if (traversals != 1) {
    free(decodedFrame.framePtr);
    return (Frame){};
  }
  return decodedFrame;
}
