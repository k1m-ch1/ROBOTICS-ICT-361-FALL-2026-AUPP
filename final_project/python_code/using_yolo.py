import os, time, cv2
from threading import Thread, Lock
from ultralytics import YOLO
import mpeg_server
import threading
from queue import Queue
from serial import Serial
import cobs

MODEL_PATH = "../../yolo_model/weights/best.onnx"

CAM_INDEX  = 0
IMG_SIZE   = 320
CONF       = 0.5

IMAGE_WIDTH = 640
IMAGE_HEIGHT = 480

MAIN_MPEG_SERVER_PORT = 8080
RAW_MPEG_SERVER_PORT = 8000

model = YOLO(MODEL_PATH)
if __name__ == "__main__":
    ser = Serial(port="/dev/ttyUSB0", baudrate=115200)
    packet = cobs.packet_constructor("SetServoAngle", (60, ))
    ser.write(packet + bytes([0x00]))

    cap = cv2.VideoCapture(0)
    cap.set(cv2.CAP_PROP_FRAME_WIDTH, IMAGE_WIDTH)
    cap.set(cv2.CAP_PROP_FRAME_HEIGHT, IMAGE_HEIGHT)
    frame_queue = Queue()
    raw_frame_queue = Queue()

    web_server_thread = threading.Thread(target=mpeg_server.web_server,
                                        args=(frame_queue, MAIN_MPEG_SERVER_PORT))
    web_server_thread.start()

    raw_web_server_thread = threading.Thread(target=mpeg_server.web_server,
                                        args=(raw_frame_queue, RAW_MPEG_SERVER_PORT))
    raw_web_server_thread.start()

    while True:
        ret, frame = cap.read()

        if not ret:
            break

        if raw_frame_queue.qsize() == 0:
            raw_frame_queue.put(frame)
        
        #results = model.predict(frame, imgsz=IMG_SIZE, conf=CONF, verbose=False)
        #frame = results[0].plot()
        #if frame_queue.qsize() == 0:
        #    frame_queue.put(frame)

