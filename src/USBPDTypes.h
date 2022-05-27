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

#endif // USBPD_TYPES_H
