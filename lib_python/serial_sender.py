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

    packet = cobs.packet_constructor("SetSpeedLimit", (0.1, 0.1))
    ser.write(packet + bytes([0x00]))

    for i in range(100):
        packet = cobs.packet_constructor("SetSpeed", (i/100, 0))
        ser.write(packet + bytes([0x00]))
        packet = cobs.packet_constructor("SetServoAngle", (70 + (i/100)*(140 - 70),))
        ser.write(packet + bytes([0x00]))
        time.sleep(0.1)

    packet = cobs.packet_constructor("SetSpeed", (0, 0))
    ser.write(packet + bytes([0x00]))
    serial_reader_thread.join()
