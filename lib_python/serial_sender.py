import serial
import struct
import cobs
import threading
import serial_reader
import time

SERIAL_PORT = "/dev/ttyUSB0"
BAUD_RATE = 115200


if __name__ == "__main__":
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE)
    serial_reader_thread = threading.Thread(target=serial_reader.serial_reader, args=(ser,))
    serial_reader_thread.start()
    packet = cobs.packet_constructor("SetSpeed", (0.11, 0.12))
    ser.write(cobs.c_frame_deconstructor(packet))
    ser.write(bytes([0x00]))
    serial_reader_thread.join()
