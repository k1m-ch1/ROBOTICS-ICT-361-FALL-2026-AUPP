# Installation and usage

## On the ESP32

Upload the firmware onto the esp32.

To do this, `cd` into the `esp32_robot_through_serial` directory and then run:

```
./upload ./sketch/sketch.ino /dev/ttyUSB0
```

On the raspberry pi (or connect the ESP32 to your computer and upload the firmware onto the ESP32)

## On the raspberry pi

To run the program, first create a virtual environment:

```
python3 -m venv .venv
```

Activate it

```
source .venv/bin/activate
```

Install the necessary packages

```
pip install opencv-python pyserial
```

Run the program:

```
python3 ./python_code/main.py
```

Now visit main port `8080` for the main view from the camera, and port `8888` for the HSV masked view from the camera.

[Video link](https://youtu.be/G1a3E9_W4Bw?si=uM4cF6UC3GrPz9IA)

# Hardware

- Custom ESP32 board with TB6612FNG motor driver
- 12V brushed DC motors
- Raspberry pi 5 8Gb model
- USB camera
