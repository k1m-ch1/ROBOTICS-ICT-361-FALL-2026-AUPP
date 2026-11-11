import os, time, cv2
from threading import Thread, Lock
from ultralytics import YOLO
import mpeg_server
import threading

MODEL_PATH = "../../yolo_model/weights/best.onnx"

CAM_INDEX  = 0
IMG_SIZE   = 320
CONF       = 0.5

IMAGE_WIDTH = 640
IMAGE_HEIGHT = 480

MAIN_MPEG_SERVER_PORT = 8000

model = YOLO(MODEL_PATH)
def gen_mjpeg():
    while True:
        ok, frame = cam.read()
        if not ok or frame is None:
            time.sleep(0.02)
            continue
        results = model.predict(frame, imgsz=IMG_SIZE, conf=CONF, verbose=False)
        annotated = results[0].plot()
        # optional resize to cut bandwidth/CPU:
        # annotated = cv2.resize(annotated, (640, 360))
        ok, jpg = cv2.imencode(".jpg", annotated, [cv2.IMWRITE_JPEG_QUALITY, 80])
        if not ok:
            continue
        b = jpg.tobytes()
        yield (b"--frame\r\n"
               b"Content-Type: image/jpeg\r\n"
               b"Cache-Control: no-cache\r\n"
               b"Content-Length: " + str(len(b)).encode() + b"\r\n\r\n" + b + b"\r\n")

if __name__ == "__main__":
    cap = cv2.VideoCapture(0)
    cap.set(cv2.CAP_PROP_FRAME_WIDTH, IMAGE_WIDTH)
    cap.set(cv2.CAP_PROP_FRAME_HEIGHT, IMAGE_HEIGHT)
    frame_queue = Queue()
    
    while True:
        web_server_thread = threading.Thread(target=mpeg_server.web_server,
                                         args=(frame_queue, MAIN_MPEG_SERVER_PORT))
        web_server_thread.start()

        ret, frame = cap.read()
        if not ret:
            break
        results = model.predict(frame, imgsz=IMG_SIZE, conf=CONF, verbose=False)
        frame = results[0].plot()
        if len(frame_queue) == 0:
            frame_queue.put(frame)






