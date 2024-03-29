#include "USBPDSimulationDataGenerator.h"

#include <AnalyzerHelpers.h>

#include "USBPDAnalyzerSettings.h"
#include "USBPDTypes.h"
#include "crc32.h"

USBPDSimulationDataGenerator::USBPDSimulationDataGenerator()
    : mSerialText("My first analyzer, woo hoo!"),
      mStringIndex(0) {}

USBPDSimulationDataGenerator::~USBPDSimulationDataGenerator() {}

void USBPDSimulationDataGenerator::Initialize(U32 simulation_sample_rate,
                                              USBPDAnalyzerSettings* settings) {
  mSimulationSampleRateHz = simulation_sample_rate;
  mSettings = settings;

  mSerialSimulationData.SetChannel(mSettings->mInputChannel);
  mSerialSimulationData.SetSampleRate(simulation_sample_rate);
  mSerialSimulationData.SetInitialBitState(BIT_HIGH);
}

U32 USBPDSimulationDataGenerator::GenerateSimulationData(
    U64 largest_sample_requested,
    U32 sample_rate,
    SimulationChannelDescriptor** simulation_channel) {
  U64 adjusted_largest_sample_requested =
      AnalyzerHelpers::AdjustSimulationTargetSample(largest_sample_requested,
                                                    sample_rate,
                                                    mSimulationSampleRateHz);

  while (mSerialSimulationData.GetCurrentSampleNumber() < (adjusted_largest_sample_requested + 3)) {
    for (int i = 0; i < NUM_SOP_TYPE; i++) {
      uint8_t portPowerRoleOrCablePlug = 0;

      if ((SOPType)i == SOPType_SOP) {
        portPowerRoleOrCablePlug = (uint8_t)PortPowerRole_Source;
      } else {
        portPowerRoleOrCablePlug = (uint8_t)CablePlug_MsgSrcPort;
      }

      CreateUSBPDControlMessageTransaction((SOPType)i,
                                           ControlMessage_Ping,
                                           PortDataRole_DFP,
                                           PDSpecRevision_2P0,
                                           portPowerRoleOrCablePlug);
    }
  }

  *simulation_channel = &mSerialSimulationData;
  return 1;
}

void USBPDSimulationDataGenerator::CreateSerialByte() {
  U32 samples_per_bit = mSimulationSampleRateHz / mSettings->mBitRate;

  U8 byte = mSerialText[mStringIndex];
  mStringIndex++;
  if (mStringIndex == mSerialText.size()) mStringIndex = 0;

  // we're currenty high
  // let's move forward a little
  mSerialSimulationData.Advance(samples_per_bit * 10);

  mSerialSimulationData.Transition();              // low-going edge for start bit
  mSerialSimulationData.Advance(samples_per_bit);  // add start bit time

  U8 mask = 0x1 << 7;
  for (U32 i = 0; i < 8; i++) {
    if ((byte & mask) != 0)
      mSerialSimulationData.TransitionIfNeeded(BIT_HIGH);
    else
      mSerialSimulationData.TransitionIfNeeded(BIT_LOW);

    mSerialSimulationData.Advance(samples_per_bit);
    mask = mask >> 1;
  }

  mSerialSimulationData.TransitionIfNeeded(BIT_HIGH);  // we need to end high

  // lets pad the end a bit for the stop bit:
  mSerialSimulationData.Advance(samples_per_bit);
}

void USBPDSimulationDataGenerator::CreateBiphaseMarkCodingBit(bool bit) {
  U32 samples_per_bit = mSimulationSampleRateHz / mSettings->mBitRate;
  U32 samples_per_transition =
      mSimulationSampleRateHz / (mSettings->mBitRate * 2);  // Two transitions per bit

  // All bits start with a transition
  mSerialSimulationData.Transition();

  // Advance halfway through the bit
  mSerialSimulationData.Advance(samples_per_transition);

  // If bit is 1, we need to transition 1/2 way through this bit
  if (bit) {
    mSerialSimulationData.Transition();
  }

  mSerialSimulationData.Advance(samples_per_transition);
}

void USBPDSimulationDataGenerator::CreateFiveBitFromFourBit(uint8_t fourBit) {
  uint8_t fiveBit = FourBitToFiveBitEncoder(fourBit);
  for (int i = 0; i < 5; i++) {
    // Always transmit LSB first
    CreateBiphaseMarkCodingBit(fiveBit & 0x1);
    fiveBit >>= 1;
  }
}

void USBPDSimulationDataGenerator::CreateByte(uint8_t byte) {
  // A byte is 2x four-bit numbers
  // Transmit LSB first
  CreateFiveBitFromFourBit(byte & 0xF);
  CreateFiveBitFromFourBit((byte >> 4) & 0xF);
}

void USBPDSimulationDataGenerator::CreatePreamble() {
  // Always start with transmitting a 0
  bool bit = false;

  for (int i = 0; i < 64; i++) {
    CreateBiphaseMarkCodingBit(bit);
    bit = !bit;
  }
}

void USBPDSimulationDataGenerator::CreateSOP(SOPType sop) {
  if (sop >= NUM_SOP_TYPE) {
    return;
  }

  const KCODEType* kcodesInSop = sop_map[sop];

  for (int i = 0; i < numKcodeInSOP; i++) {
    KCODEType currentKcode = kcodesInSop[i];
    CreateKCode(currentKcode);
  }
}

void USBPDSimulationDataGenerator::CreateKCode(KCODEType code) {
  if (code >= NUM_KCODE) {
    return;
  }

  uint8_t kcode_value = kcode_map[code];
  for (int i = 0; i < numKcodeBits; i++) {
    CreateBiphaseMarkCodingBit(kcode_value & 0x1);
    kcode_value >>= 1;
  }
}

// Take a 4-bit number as input and produce a 5-bit number as output
// Used for message encoding on-the-wire
uint8_t USBPDSimulationDataGenerator::FourBitToFiveBitEncoder(uint8_t val) {
  return fourBitToFiveBitLUT[(val & 0x0F)];
}

uint16_t USBPDSimulationDataGenerator::CreateMessageHeader(uint8_t messageType,
                                                           uint8_t portDataRole,
                                                           uint8_t specificationRevision,
                                                           uint8_t portPowerRoleOrCablePlug,
                                                           uint8_t messageId,
                                                           uint8_t numOfDataObjects) {
  uint16_t nibble0 = 0;  // [3..0: Message Type]
  uint16_t nibble1 = 0;  // [7..6: Specification Revision] [5: Port Data Role if SOP / Reserved, 0
                         // if SOP' or SOP"] [4: Reserved, 0]
  uint16_t nibble2 =
      0;  // [11..9: Message ID] [8: Port Power Role if SOP / Cable Plug if SOP' or SOP"]
  uint16_t nibble3 = 0;  // [15: Reserved, 0] [14..12: Number of Data Objects]

  nibble0 = messageType & 0xF;

  // nibble1 bit 0 is reserved
  nibble1 |= (portDataRole & 0x1) << 1;
  nibble1 |= (specificationRevision & 0x3) << 2;

  nibble2 |= (portPowerRoleOrCablePlug & 0x1);
  nibble2 |= (messageId & 0x7) << 1;

  nibble3 |= (numOfDataObjects & 0x7);

  CreateFiveBitFromFourBit((uint8_t)nibble0);
  CreateFiveBitFromFourBit((uint8_t)nibble1);
  CreateFiveBitFromFourBit((uint8_t)nibble2);
  CreateFiveBitFromFourBit((uint8_t)nibble3);

  return ((nibble3 & 0xF) << 12) | ((nibble2 & 0xF) << 8) | ((nibble1 & 0xF) << 4) |
         (nibble0 & 0xF);
}

uint16_t USBPDSimulationDataGenerator::CreateControlMessageHeader(ControlMessageTypes type,
                                                                  PortDataRole role,
                                                                  PDSpecRevision rev,
                                                                  uint8_t portPowerRoleOrCablePlug,
                                                                  uint8_t messageId) {
  return CreateMessageHeader((uint8_t)type,
                             (uint8_t)role,
                             (uint8_t)rev,
                             portPowerRoleOrCablePlug,
                             messageId,
                             0);  // Control messages always have 0 data objects
}

void CreateDataMessageHeader() {}

// Simulate a system with all possible communicators
// UFP and DFP as well as both cable ends

uint8_t ufpMessageId = 0;
uint8_t dfpMessageId = 0;
uint8_t sopPrimeCableMessageId = 0;
uint8_t sopDoublePrimeCableMessageId = 0;

void USBPDSimulationDataGenerator::CreateUSBPDControlMessageTransaction(
    SOPType sop,
    ControlMessageTypes messageType,
    PortDataRole dataRole,
    PDSpecRevision specRev,
    uint8_t portPowerRoleOrCablePlug) {
  U32 samples_per_bit = mSimulationSampleRateHz / mSettings->mBitRate;

  // Ensure we start in the high state
  // TODO: simulator settings that allow us to start in either the high or low state: either
  // state is technically valid for Biphase Mark Coding
  mSerialSimulationData.TransitionIfNeeded(BIT_HIGH);

  // we're currenty high
  // let's move forward a little
  mSerialSimulationData.Advance(samples_per_bit * 10);

  CreatePreamble();

  CreateSOP(sop);

  uint8_t* sendMessageId = NULL;
  uint8_t* replyMessageId = NULL;

  if (sop == SOPType_SOP) {
    // Who is sending this message?
    if (dataRole == PortDataRole_UFP) {
      sendMessageId = &ufpMessageId;
    } else {
      sendMessageId = &dfpMessageId;
    }

  } else {
    // We are talking to the cable since we are not using SOP
    // Cables can never initialte a transaction, therefore this must
    // come from the UFP or the DFP
    // There's no way to tell who is sending the message to the cable,
    // So lets just assume it's the DFP
    sendMessageId = &dfpMessageId;

    // If message type is not a SOP message, then
    // PortDataRole must be 0
    dataRole = (PortDataRole)0;
  }

  uint16_t headerBytes = CreateControlMessageHeader(messageType,
                                                    dataRole,
                                                    specRev,
                                                    portPowerRoleOrCablePlug,
                                                    *sendMessageId);

  *sendMessageId++;
  if (*sendMessageId > 7) {
    *sendMessageId = 0;
  }

  // Send CRC32
  uint32_t crc = crc32(0x00000000, (const uint8_t*)&headerBytes, sizeof(uint16_t), usbCrcPolynomial);

  CreateByte(crc & 0xFF);
  CreateByte((crc >> 8) & 0xFF);
  CreateByte((crc >> 16) & 0xFF);
  CreateByte((crc >> 24) & 0xFF);

  // Send EOP sequence
  CreateKCode(KCODEType_EOP);

  // All Frames end with a final edge transition
  mSerialSimulationData.Transition();

  mSerialSimulationData.Advance(samples_per_bit * 10);

  // Reply with a GoodCRC message from the other side
  CreatePreamble();

  CreateSOP(sop);

  if (sop == SOPType_SOP) {
    // If we are replying to a SOP message, we should swap port data roles
    if (dataRole == PortDataRole_UFP) {
      dataRole = PortDataRole_DFP;
      replyMessageId = &dfpMessageId;
    } else {
      dataRole = PortDataRole_UFP;
      replyMessageId = &ufpMessageId;
    }

    // If we are replying to a SOP message, we should swap port power roles
    if (portPowerRoleOrCablePlug == PortPowerRole_Sink) {
      portPowerRoleOrCablePlug = PortPowerRole_Source;
    } else {
      portPowerRoleOrCablePlug = PortPowerRole_Sink;
    }
  } else {
    // Replying to a SOP' or SOP" message, dataRole must be 0
    dataRole = (PortDataRole)0;

    // If we are replying to a SOP' or SOP" message, we must be the cable
    portPowerRoleOrCablePlug = CablePlug_MsgSrcPlug;

    if (sop == SOPType_SOP_PRIME || sop == SOPType_SOP_PRIME_DEBUG) {
      replyMessageId = &sopPrimeCableMessageId;
    } else {
      replyMessageId = &sopDoublePrimeCableMessageId;
    }
  }

  headerBytes = CreateControlMessageHeader(ControlMessage_GoodCRC,
                                           dataRole,
                                           specRev,
                                           portPowerRoleOrCablePlug,
                                           *replyMessageId);

  *replyMessageId++;
  if (*replyMessageId > 7) {
    *replyMessageId = 0;
  }

  // Send CRC32
  crc = crc32(0x00000000, (const uint8_t*)&headerBytes, sizeof(uint16_t), usbCrcPolynomial);

  CreateByte(crc & 0xFF);
  CreateByte((crc >> 8) & 0xFF);
  CreateByte((crc >> 16) & 0xFF);
  CreateByte((crc >> 24) & 0xFF);

  // Send EOP sequence
  CreateKCode(KCODEType_EOP);
}
