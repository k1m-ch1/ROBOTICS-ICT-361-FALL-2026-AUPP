def serial_reader(ser):
    # serial reader is a thread
    while True:
        receivedByte = ser.read(1)
        print(receivedByte.decode("ascii"), end="", flush=True)
