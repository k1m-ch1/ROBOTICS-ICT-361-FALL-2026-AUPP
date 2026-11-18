import cv2
import time
from enum import Enum
import math
from dataclasses import dataclass
from queue import Queue

import serial
import serial_reader
import threading
import cobs
import mpeg_server
from line_follow_utils import *
from pure_pursuit import get_normalized_angular_effort

SERIAL_PORT = "/dev/ttyUSB0"
BAUD_RATE = 115200

FRAME_RATE = 30
MAIN_MPEG_SERVER_PORT = 8080
MASKED_MPEG_SERVER_PORT = 8888

HSV_LOWER_BOUND = (0, 0, 0)
HSV_UPPER_BOUND = (179, 255, 150)

IMAGE_WIDTH = 640
IMAGE_HEIGHT = 480

MAX_LINEAR_SPEED = 0.04
MAX_ANGULAR_SPEED = MAX_LINEAR_SPEED

PIXELS_PER_CM = IMAGE_WIDTH/5

# this is approximately the distance between the center of mass of the car to the start of the image seen in the camera
CAMERA_VERTICAL_OFFSET_FROM_CM = 3
MOTOR_TO_CM = 8

TOP_IMAGE_TEXT_COORD = (20, 20)

# this value proportion over the mask at which we say that anything
# below this proportion will be considered to have no line in the image
NO_LINE_PROPORTION = 0.05

def draw_text(frame, text, coordinates):
    cv2.putText(
        frame,
        text,
        coordinates,
        cv2.FONT_HERSHEY_SIMPLEX,
        1,
        (0, 255, 0),
        2
    )


def main():
    frame_queue = Queue()
    masked_frame_queue = Queue()

    web_server_thread = threading.Thread(target=mpeg_server.web_server,
                                         args=(frame_queue, MAIN_MPEG_SERVER_PORT))
    web_server_thread.start()

    masked_web_server_thread = threading.Thread(target=mpeg_server.web_server,
                                                args=(masked_frame_queue, MASKED_MPEG_SERVER_PORT))
    masked_web_server_thread.start()

    ser = serial.Serial(SERIAL_PORT, BAUD_RATE)
    serial_reader_thread = threading.Thread(target=serial_reader.serial_reader,
                                            args=(ser,))
    serial_reader_thread.start()

    # set the speed limit initially
    packet = cobs.packet_constructor("SetSpeedLimit", (1, 1))
    ser.write(packet + bytes([0x00]))

    packet = cobs.packet_constructor("SetSpeed", (0, 0))
    ser.write(packet + bytes([0x00]))

    packet = cobs.packet_constructor("SetServoAngle", (60,))
    ser.write(packet + bytes([0x00]))


    cap = cv2.VideoCapture(0)
    cap.set(cv2.CAP_PROP_FRAME_WIDTH, IMAGE_WIDTH)
    cap.set(cv2.CAP_PROP_FRAME_HEIGHT, IMAGE_HEIGHT)

    while True:
        ret, frame = cap.read()
        if not ret:
            break

        rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)


        #cv2.imshow("determining action", frame)
        masked_frame = cv2.inRange(hsv, HSV_LOWER_BOUND, HSV_UPPER_BOUND)
        moments = cv2.moments(masked_frame)
        if moments['m00'] / (255*frame.shape[0]*frame.shape[1]) > NO_LINE_PROPORTION:
            x_cm = int(moments['m10']/moments['m00'])
            y_cm = int(moments['m01']/moments['m00'])

            cv2.circle(
                frame,
                (x_cm, y_cm),
                8,
                (0, 0, 255),
                -1
            )

            actual_coordinates = image_pixel_to_coordinates_cm(
                                                    x_pixel = x_cm,
                                                    y_pixel = y_cm,
                                                    image_width = IMAGE_WIDTH,
                                                    image_height = IMAGE_HEIGHT,
                                                    pixel_per_cm = PIXELS_PER_CM
                                                )

            actual_coordinates = (actual_coordinates[0], actual_coordinates[1] + CAMERA_VERTICAL_OFFSET_FROM_CM)

            draw_text(
                frame=frame,
                text=f"{actual_coordinates}",
                coordinates=(x_cm, y_cm),
            )

            angular_speed = get_normalized_angular_effort(
                actual_coordinates[0],
                actual_coordinates[1],
                MOTOR_TO_CM
            ) * MAX_ANGULAR_SPEED

            linear_speed = MAX_LINEAR_SPEED

            draw_text(
                frame=frame,
                text=f"l: {linear_speed}, a: {angular_speed}",
                coordinates=TOP_IMAGE_TEXT_COORD,
            )

            packet = cobs.packet_constructor("SetSpeed", (linear_speed, angular_speed))
            ser.write(packet + bytes([0x00]))

        else:
            packet = cobs.packet_constructor("SetSpeed", (0, 0))
            ser.write(packet + bytes([0x00]))
            draw_text(
                frame=frame,
                text=f"no line detected",
                coordinates=TOP_IMAGE_TEXT_COORD,
            )

        # put new frames into the queue only if it has already been consumed
        if frame_queue.qsize() == 0:
            frame_queue.put(frame)

        if masked_frame_queue.qsize() == 0:
            masked_frame_queue.put(masked_frame)
        
        #if cv2.waitKey(1) & 0xFF == ord('q'):
        #    break

    cap.release()
    cv2.destroyAllWindows()


if __name__ == "__main__":
    main()

