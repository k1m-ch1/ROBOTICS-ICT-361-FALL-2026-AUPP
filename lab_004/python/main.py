import cv2
import mediapipe as mp
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

finger_string_to_index = {
    "index": 0,
    "middle": 1,
    "ring": 2,
    "pinkie": 3
}

mp_hands = mp.solutions.hands
mp_drawing = mp.solutions.drawing_utils

class States(Enum):
    STOP = 0
    FORWARD = 1
    BACKWARD = 2
    LEFT = 3
    RIGHT = 4

@dataclass
class Action:
    geasture: str
    state: States

def determine_action(hand_landmarks):
    """
    Determine the action based on the observed geasture
    """
    lm = hand_landmarks.landmark

    finger_tips = [
        mp_hands.HandLandmark.INDEX_FINGER_TIP,
        mp_hands.HandLandmark.MIDDLE_FINGER_TIP,
        mp_hands.HandLandmark.RING_FINGER_TIP,
        mp_hands.HandLandmark.PINKY_TIP
    ]

    finger_pips = [
        mp_hands.HandLandmark.INDEX_FINGER_PIP,
        mp_hands.HandLandmark.MIDDLE_FINGER_PIP,
        mp_hands.HandLandmark.RING_FINGER_PIP,
        mp_hands.HandLandmark.PINKY_PIP
    ]

    finger_mcp = [
        mp_hands.HandLandmark.INDEX_FINGER_MCP,
        mp_hands.HandLandmark.MIDDLE_FINGER_MCP,
        mp_hands.HandLandmark.RING_FINGER_MCP,
        mp_hands.HandLandmark.PINKY_MCP
    ]

    thumb_up = False

    fingers_up = []

    wrist_to_tip_vector = []

    wrist_to_pip_vector = []

    # we apply a special condition for thumb, we use the heuristic where we compare the distance from the thumb tip to the index finger MCP instead
    index_mcp_to_thumb_tip_distance = math.hypot(lm[mp_hands.HandLandmark.THUMB_TIP].x - lm[mp_hands.HandLandmark.INDEX_FINGER_MCP].x,
                                                 lm[mp_hands.HandLandmark.THUMB_TIP].y - lm[mp_hands.HandLandmark.INDEX_FINGER_MCP].y)
    index_mcp_to_thumb_ip_distance = math.hypot(lm[mp_hands.HandLandmark.THUMB_IP].x - lm[mp_hands.HandLandmark.INDEX_FINGER_MCP].x,
                                                 lm[mp_hands.HandLandmark.THUMB_IP].y - lm[mp_hands.HandLandmark.INDEX_FINGER_MCP].y)


    thumb_up = index_mcp_to_thumb_tip_distance > index_mcp_to_thumb_ip_distance
    for tip_id, pip_id in zip(finger_tips, finger_pips):
        # for each tip, we compute the vector between the MCP to the tip and the MCP to the pip
        wrist_to_pip_vector.append((lm[pip_id].x - lm[0].x, lm[pip_id].y - lm[0].y))
        wrist_to_tip_vector.append((lm[tip_id].x - lm[0].x, lm[tip_id].y - lm[0].y))
        distance_wrist_to_pip = math.hypot(*wrist_to_pip_vector[-1])
        distance_wrist_to_tip = math.hypot(*wrist_to_tip_vector[-1])
        
        # if the distance from the wrist to the tip is greater than the distance from the wrist to the pip, we consider it as finger up
        fingers_up.append(distance_wrist_to_tip > distance_wrist_to_pip)

    # the first condition is to check whether it's an open palm, to do so, we just check 
    is_open_palm = thumb_up and all(fingers_up)
    is_closed_palm = not thumb_up and all([not finger for finger in fingers_up])

    # for the v sign, we must check that only the index and the middle finger is up
    is_v_sign = fingers_up[finger_string_to_index["index"]]\
    and fingers_up[finger_string_to_index["middle"]]\
    and not fingers_up[finger_string_to_index["ring"]]\
    and not fingers_up[finger_string_to_index["pinkie"]]

    # for the index finger pointing up, we must check that only the index finger is up (also, the thumb position is hard to close when we're doing a v)
    is_index_only = fingers_up[finger_string_to_index["index"]]\
    and not fingers_up[finger_string_to_index["middle"]]\
    and not fingers_up[finger_string_to_index["ring"]]\
    and not fingers_up[finger_string_to_index["pinkie"]]

    if (is_open_palm):
        # first check whether it is an open palm. if yes, stop
        return Action(geasture="open palm",
                      state=States.STOP)
    elif (is_closed_palm):
        # if it is a closed palm, go backwards
        return Action(geasture="fist",
                      state=States.BACKWARD)
    elif (is_v_sign):
        return Action(geasture="v sign",
                      state=States.FORWARD)
    elif (is_index_only):
        # if it is index only, divide into two cases. we must now calculate the angle by doing atan2
        angle = math.degrees(math.atan2(-wrist_to_tip_vector[finger_string_to_index["index"]][1], wrist_to_tip_vector[finger_string_to_index["index"]][0]))
        
        # if the angle is in between [90, 180] and [-90, -180] consider it as left
        # otherwise, if it's in between [0, 90] and [-90, 0] consider it as right
        # we can more elegantly state this by computing |alpha| > 90 as left and |alpha| < 90 as right
        if math.fabs(angle) > 90:
            return Action(geasture="index pointing left",
                          state=States.LEFT)
        elif math.fabs(angle) <= 90:
            return Action(geasture="index pointing right",
                          state=States.RIGHT)
    
    return Action(
        geasture="unknown geasutre (force stop)",
        state=States.STOP
    )



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

    cap = cv2.VideoCapture(0)
    cap.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
    cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)

    # only capture one hand at a time
    with mp_hands.Hands(
        max_num_hands=1,
        model_complexity=1,
        min_detection_confidence=0.5,
        min_tracking_confidence=0.5
    ) as hands:

        while True:
            ret, frame = cap.read()
            if not ret:
                break

            frame = cv2.flip(frame, 1)
            rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
            results = hands.process(rgb)

            if results.multi_hand_landmarks:
                for hand_landmarks, handedness in zip(
                        results.multi_hand_landmarks,
                        results.multi_handedness):

                    mp_drawing.draw_landmarks(
                        frame,
                        hand_landmarks,
                        mp_hands.HAND_CONNECTIONS
                    )

                    action = determine_action(hand_landmarks)
                    print(action.geasture)

                    cv2.putText(
                        frame,
                        f"""Geasture: {action.geasture}
                        Action: {action.state.name}
                        """,
                        (10, 60),
                        cv2.FONT_HERSHEY_SIMPLEX,
                        1.5,
                        (0, 255, 0),
                        3
                    )
                    if (action.state == States.STOP):
                        packet = cobs.packet_constructor("SetSpeed", (0, 0))
                        ser.write(packet + bytes([0x00]))
                    elif (action.state == States.FORWARD):
                        packet = cobs.packet_constructor("SetSpeed", (1, 0))
                        ser.write(packet + bytes([0x00]))
                    elif (action.state == States.BACKWARD):
                        packet = cobs.packet_constructor("SetSpeed", (-1, 0))
                        ser.write(packet + bytes([0x00]))
                    elif (action.state == States.LEFT):
                        packet = cobs.packet_constructor("SetSpeed", (0, 1))
                        ser.write(packet + bytes([0x00]))
                    elif (action.state == States.RIGHT):
                        packet = cobs.packet_constructor("SetSpeed", (0, -1))
                        ser.write(packet + bytes([0x00]))
            else:
                # force stop if no hand is detected
                packet = cobs.packet_constructor("SetSpeed", (0, 0))
                ser.write(packet + bytes([0x00]))

            #cv2.imshow("determining action", frame)
            frame_queue.put(frame)
            if cv2.waitKey(1) & 0xFF == ord('q'):
                break

    cap.release()
    cv2.destroyAllWindows()


if __name__ == "__main__":
    main()

