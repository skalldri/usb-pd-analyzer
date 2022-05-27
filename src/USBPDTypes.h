#ifndef USBPD_TYPES_H
#define USBPD_TYPES_H

enum FrameType {
  FRAME_TYPE_PREAMBLE,
  FRAME_TYPE_SOP,
  FRAME_TYPE_SOP_PRIME,
  FRAME_TYPE_SOP_DOUBLE_PRIME,
  FRAME_TYPE_SOP_PRIME_DEBUG,
  FRAME_TYPE_SOP_DOUBLE_PRIME_DEBUG,
  FRAME_TYPE_SOP_ERROR,
  FRAME_TYPE_HEADER,

  NUM_FRAME_TYPE
};

enum KCODE {
  SYNC_1,
  SYNC_2,
  RST_1,
  RST_2,
  EOP,
  SYNC_3,

  NUM_KCODE
};

static const int numKcodeBits = 5;
static const uint8_t kcode_map[NUM_KCODE] = {
    0x18, // Sync-1 = 11000
    0x11, // Sync-2 = 10001
    0x07, // RST-1  = 00111
    0x19, // RST-2  = 11001
    0x0D, // EOP    = 01101
    0x06, // Sync-3 = 00110
};

enum SOPTypes {
  SOP,
  SOP_PRIME,
  SOP_DOUBLE_PRIME,
  SOP_PRIME_DEBUG,
  SOP_DOUBLE_PRIME_DEBUG,

  NUM_SOP_TYPE
};

const int numKcodeInSOP = 4;
static const KCODE sop_map[NUM_SOP_TYPE][numKcodeInSOP] = {
    {SYNC_1, SYNC_1, SYNC_1, SYNC_2}, // SOP
    {SYNC_1, SYNC_1, SYNC_3, SYNC_3}, // SOP'
    {SYNC_1, SYNC_3, SYNC_1, SYNC_3}, // SOP"
    {SYNC_1, RST_2,  RST_2,  SYNC_3}, // SOP' Debug
    {SYNC_1, RST_2,  SYNC_3, SYNC_2}, // SOP" Debug
};

static const uint8_t fourBitToFiveBitLUT[16] = {
    // case 0x0:
    0x1E, // 11110

    // case 0x1:
    0x09, // 01001

    // case 0x2:
    0x14, // 10100

    // case 0x3:
    0x15, // 10101

    // case 0x4:
    0x0A, // 01010

    // case 0x5:
    0x0B, // 01011

    // case 0x6:
    0x0E, // 01110

    // case 0x7:
    0x0F, // 01111

    // case 0x8:
    0x12, // 10010

    // case 0x9:
    0x13, // 10011

    // case 0xA:
    0x16, // 10110

    // case 0xB:
    0x17, // 10111

    // case 0xC:
    0x1A, // 11010

    // case 0xD:
    0x1B, // 11011

    // case 0xE:
    0x1C, // 11100

    // case 0xF:
    0x1D, // 11101
};

enum ControlMessageTypes {
    ControlMessage_Reserved,
    ControlMessage_GoodCRC,
    ControlMessage_GotoMin,
    ControlMessage_Accept,
    ControlMessage_Reject,
    ControlMessage_Ping,
    ControlMessage_PS_RDY,
    ControlMessage_Get_Source_Cap,
    ControlMessage_Get_Sink_Cap,
    ControlMessage_DR_Swap,
    ControlMessage_PR_Swap,
    ControlMessage_VCONN_Swap,
    ControlMessage_Wait,
    ControlMessage_Soft_Reset,

    ControlMessage_Reserved14,
    ControlMessage_Reserved15,

    NUM_CONTROL_MESSAGE
};

enum DataMessageTypes {
    DataMessage_Reserved,
    DataMessage_Source_Capabilities,
    DataMessage_Request,
    DataMessage_BIST,
    DataMessage_Sink_Capabilities,

    DataMessage_Reserved5,
    DataMessage_Reserved6,
    DataMessage_Reserved7,
    DataMessage_Reserved8,
    DataMessage_Reserved9,
    DataMessage_Reserved10,
    DataMessage_Reserved11,
    DataMessage_Reserved12,
    DataMessage_Reserved13,
    DataMessage_Reserved14,

    DataMessage_Vendor_Defined,

    NUM_DATA_MESSAGE
};

#endif // USBPD_TYPES_H
