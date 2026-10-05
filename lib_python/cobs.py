import ctypes
import packets
import struct


cobs = ctypes.CDLL("./libcobs.so")
class Frame(ctypes.Structure):
    _fields_ = [
        ("framePtr", ctypes.POINTER(ctypes.c_uint8)),
        ("size", ctypes.c_uint32),
    ]

cobs.cobsEncode.argtypes = [Frame]
cobs.cobsEncode.restype = Frame

#data = bytes([0x11, 0x22, 0x00, 0x33])
#buffer = (ctypes.c_uint8*len(data)).from_buffer_copy(data)
#print(type(buffer))
#frame = Frame(
#    buffer,
#    len(data)
#)
#
#encoded = cobs.cobsEncode(frame)
#print(encoded.size)
#print(bytes(encoded.framePtr[:encoded.size]).hex(' '))

def c_frame_constructor(frame_as_bytes):
    buffer = (ctypes.c_uint8*len(frame_as_bytes)).from_buffer_copy(frame_as_bytes)
    frame = Frame(
        buffer,
        len(frame_as_bytes)
    )
    return frame

def c_frame_deconstructor(frame):
    return ctypes.string_at(frame.framePtr, frame.size)

def packet_constructor(packet_name, args):
    # now we need to send a struct
    packet_id = packets.NAME_TO_PACKET_ID[packet_name]
    packet_format = packets.PACKET_ID_TO_STRUCT[packet_id]
    packed_struct = struct.pack(packet_format, packet_id, *args)
    raw_frame = c_frame_constructor(packed_struct)
    encoded_frame = cobs.cobsEncode(raw_frame)
    return encoded_frame
