#pragma once
#include <stdint.h>

typedef struct __attribute__((packed)) {
  uint8_t packetID = 0x01;
  float linear;
  float angular;
} SetSpeed;

// we do this because we don't want padding
typedef struct __attribute__((packed)) {
  uint8_t packetID = 0x02;
  float linear;
  float angular;
} SetSpeedLimit;

typedef struct __attribute__((packed)) {
  uint8_t packetID = 0x03;
  float angle;
} SetServoAngle;
