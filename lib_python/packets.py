NAME_TO_PACKET_ID = {
    "SetSpeed" : 0x01,
    "SetSpeedLimit": 0x02,
    "SetServoAngle": 0x03
}

PACKET_ID_TO_STRUCT = {
    0x01: "<Bff",
    0x02: "<Bff",
    0x03: "<Bf",
}
