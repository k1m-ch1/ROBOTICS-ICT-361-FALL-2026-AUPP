# Usage

## Packets

TLDR: Packets are approximately structs

The idea is we send a packet, which is essentially a struct with an identification number, also called packet ID, which is a 1 byte identifier as to how we should interpret the packet.

Since a struct is just an array of bytes, we essentially will take that array of bytes, encode it using COBS in order to ensure correct interpretation of the frame because we want to use the `0x00` byte as the delimiter.

## In C

Make sure that you have:

- `cobs.c`
- `cobs.h`

This will be the main logic on how we can take an array of bytes and encode it using COBS (we call it a frame). This file is pretty much a pure encoder where it doesn't have any side effects, so it can be used as a pure function.

Now, for the implementation specific to the robot, we essentially have a few packet definitions which are just structs with the constraint that it has a packet ID as the first byte.

This is in:

- `packets.h`

```c
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
```

To add a new packet, essentially just make sure that the `packetID` you choose is unique, as such:

```c
typedef struct __attribute__((packed)) {
  uint8_t packetID = 0x03;
  float angle;
} SetServoAngle;
```

Now in our implementation right now, the ESP32 is only receiving commands, and isn't sending telemetry back, so after we add another packet, all we need to do is handle it in case someone send a packet with that particular packetID, which is done in the `frameReader.cpp` in our implementation.

In fact, `frameDecoder.cpp` is the one who sits on top of `frameReader.cpp`, but `frameDecoder.cpp`'s job is to essentially take note of the `0x00` as the delimiter and split off the packet, decode the packet into a decoded byte array representing the struct and send it to the `frameReader.cpp` which converts it to a struct and unpack it, and do stuff with it accordingly.

So if we did add a packet, all we would need to do is add an extra conditional in the switch statement to handle, in our case, the `0x03` packet.

So something like this:

```c
case 0x03: {
  if (decodedFrame.size != sizeof(SetServoAngle)) {
    break;
  }
  SetServoAngle *setServoAngle = (SetServoAngle *)decodedFrame.framePtr;
  servoWrite(setServoAngle->angle);
}
```

Essentially, `decodedFrame` is a `uint8_t` array, and we can just switch case the first byte of that array.

And that's pretty much it on how you'd handle an extra command.

## In Python

The python implementation essentially uses the `cobs.cpp` implementation of cobs encoding in order to encode raw byte arrays. Essentially, we compile the `cobs.cpp` with its header as a shared library, and then write a python wrapper around it. In our case, the wrapper is in `cobs.py` which utilizes our shared library `libcobs.so`, and we just do something like:

```python
import serial
import cobs
ser = serial.Serial(SERIAL_PORT, BAUD_RATE)
packet = cobs.packet_constructor("SetSpeed", (0, 0))
ser.write(packet + bytes([0x00]))
```

And we must define our packets in side of `packets.py`:

```python
NAME_TO_PACKET_ID = {
    "SetSpeed" : 0x01,
    "SetSpeedLimit": 0x02,
    "SetServoAngle": 0x03
}

PACKET_ID_TO_STRUCT = {
    0x01: "<Bff",
    0x02: "<Bff",
    0x03: "<Bf",
}
```

To explain the code to use it, we just send what kind of packet we want to send, and then send the parameters accordingly. Note that it's not really descriptive on what each argument represents, however, we can cross reference it in some sort of specs, or just look at the c struct definition.

In the case of `SetSpeed`, we were sending 2 floats, where the first index is for `.linear` and the second index is for `.angular`.

Now for `SetServoAngle` which we just added, we essentially just send one argument as a float.

The weird struct formatting like `<Bff` is a convention used by the builtin `struct` library in python. More about the formatting can be found [here](https://docs.python.org/3/library/struct.html).

Essentially, we use `<Bf` because, it's like little-endian, and we send one unsigned byte as a `packetID`, and we also send one float as an `angle`, and to send it, you essentially just call:

```python
import serial
import cobs
ser = serial.Serial(SERIAL_PORT, BAUD_RATE)
packet = cobs.packet_constructor("SetServoAngle", (90,))
ser.write(packet + bytes([0x00]))
```

# Details

COBS encoding is pretty simple solution to the packet framing problem.

The issue is that COBS only sorts of guarantee framing assuming that the transmission is reliable. We can obviously do some sort of rudimentary checking like, if the final pointer doesn't point to a 0x00, disregard the whole frame, but honestly, even without guarantees, something like:

- if pointer doesn't match up (especially for the last byte), destroy the whole thing

BTW, should I dynamically allocate at the decoding stage? I wish we could but if we want to make the decoder naive, that is, its job is only to take a frame in transmission and turn it into an actual frame, then we shouldn't dynamically allocate it... Should i just have like a fixed size buffer? I guess that's the simplest solution, we're not doing anything super sophisticated. I mean, the reason why we're doing this in the first place is because ASCII based protocol decoding is a bit scuff and hard reason and make it fool-proof.

- having a packet id as the first byte to encode a struct

# Implemenation

A simple decoder can be something like:

- chime in, read byte and ignore everything until we see a 0x00
- now, parse the next byte as the pointer to the next 0x00
- take that byte and do a loop that read byte by byte, or perhaps use some sort of readNBytes if the api has it
- push that into my soon to be complete struct
- add a 0x00 and push that to that struct too
- now go back to the read parse next byte as pointer to the next 0x00 stage
- ...

When encoding, add a `0x00` when we're first starting because we want to ensure the listener is synchronized (or something else because `0x00` is super common. It's ideal to use something like `0xAA` or something that doesn't really appear in the ASCII's 0 to 9, A-z, but honestly, it doesn't matter that much). I think `0xAA` is a good choice.

# Using COBS to frame structs

The idea is as follows:

- structs can be treated as a sequence of bytes, that is, we can treat it as a frame where we have an array of bytes, and we have the size of the array.


