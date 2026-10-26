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

SERIAL_PORT = "/dev/ttyUSB0"
BAUD_RATE = 115200

FRAME_RATE = 30

def main():
    frame_queue = Queue()

    web_server_thread = threading.Thread(target=mpeg_server.web_server, args=(frame_queue,))
    web_server_thread.start()

    ser = serial.Serial(SERIAL_PORT, BAUD_RATE)
    serial_reader_thread = threading.Thread(target=serial_reader.serial_reader, args=(ser,))
    serial_reader_thread.start()

    # set the speed limit initially
    packet = cobs.packet_constructor("SetSpeedLimit", (0.25, 0.25))
    ser.write(packet + bytes([0x00]))

    packet = cobs.packet_constructor("SetSpeed", (0, 0))
    ser.write(packet + bytes([0x00]))

    packet = cobs.packet_constructor("SetServoAngle", (60,))
    ser.write(packet + bytes([0x00]))


    cap = cv2.VideoCapture(0)
    cap.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
    cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)

    while True:
        ret, frame = cap.read()
        if not ret:
            break

        rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        cv2.putText(
            frame,
            "hello",
            (10, 60),
            cv2.FONT_HERSHEY_SIMPLEX,
            1.5,
            (0, 255, 0),
            3
        )

        #cv2.imshow("determining action", frame)
        frame_queue.put(frame)
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    cap.release()
    cv2.destroyAllWindows()


if __name__ == "__main__":
    main()

