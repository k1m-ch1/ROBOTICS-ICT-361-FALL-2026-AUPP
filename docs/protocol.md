We want to use the UART protocol to send the binary representation of structs from python to the ESP32, and from the ESP32 back to python.

To do this, we will use Constant Overhead Byte Stuffing encoding (COBS encoding) for packet framing.

We choose the `0x00` byte as the delimiter. Essentially, the task that's directly reading on the ESP32 is going to be the packet frame decoder, which essentially just stores a buffer, let's say that the struct will never be more than like `256` bytes, and essentially, it will just do like a read byte and then stores it into the buffer, and once we see a `0x00` byte, it will try to decode it and then it will push it to a queue of decoded frames.

The consumer of that queue of decoded frames will look at the packet id and then decide how to structure it into a struct.

We won't do explicit error detection using like checksums or even any error correction, rather, we will just to preliminary error checking based on whether it is possible to:

- decode the COBS frame in a valid manner (if the linked list doesn't make sense, then something went wrong with the encoding)
- if by coincidence, messed up data just so happened to be a valid COBS frame, check whether the packet id is even valid
- if by coincidence, the packet id is also valid, check that its size is the same as the expected struct that it maps to
- if all of that is met, just assume that it's a valid command for simplicity, and then have additional clamping like if the motor speed is like more than 1, etc.

We will just define some specs for the packets we want to send from the computer to the ESP32. We can include telemetry later on (that is, communication from the ESP32 to the computer) however, for now, we will use that channel for logging only.

So let's have:

- a set speed struct:
  - `speed.linear: float`
  - `speed.angular: float`

- a set speed limit struct:
  - `speedLimit.linear:float`
  - `speedLimit.angular:float`

- a set servo angle struct:
  - `angle: float`

We can get telemetry later if we want the ultrasonic distance. Also, the telemetry structs should also occupy a different "packet id space" similar to the minecraft classic protocol
