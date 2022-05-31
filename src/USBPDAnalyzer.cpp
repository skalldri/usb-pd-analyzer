#include "USBPDAnalyzer.h"

#include <AnalyzerChannelData.h>

#include <iostream>

#include "USBPDAnalyzerSettings.h"
#include "crc32.h"

using namespace std;

USBPDAnalyzer::USBPDAnalyzer()
    : Analyzer2(),
      mSettings(new USBPDAnalyzerSettings()),
      mSimulationInitilized(false) {
  // Generate the LUT for converting 5 bit code into 4 bit code
  for (int i = 0; i < 16; i++) {
    fiveToFourBitLUT.insert(std::make_pair(fourBitToFiveBitLUT[i], i));
  }

  SetAnalyzerSettings(mSettings.get());
}

USBPDAnalyzer::~USBPDAnalyzer() { KillThread(); }

void USBPDAnalyzer::SetupResults() {
  mResults.reset(new USBPDAnalyzerResults(this, mSettings.get()));
  SetAnalyzerResults(mResults.get());
  mResults->AddChannelBubblesWillAppearOn(mSettings->mInputChannel);
}

// Needs to start on an edge!
bool USBPDAnalyzer::ReadBiphaseMarkCodeBit() {
  U32 samples_per_bit = mSampleRateHz / mSettings->mBitRate;
  U32 samples_per_transition =
      mSampleRateHz / (mSettings->mBitRate * 2);  // Two transitions per bit

  U8 data = 0;

  // Sample number for the first edge
  U64 firstEdgeSampleNumber = mSerial->GetSampleNumber();

  mSerial->AdvanceToNextEdge();

  // Sample number for the second edge
  U64 secondEdgeSampleNumber = mSerial->GetSampleNumber();

  U64 edgeDelta = (secondEdgeSampleNumber - firstEdgeSampleNumber);

  // Detect glitches: if edgeDelta is <10% of samples_per_bit then this is probably a glitch
  if (edgeDelta <= (samples_per_bit * 0.1)) {
    cout << "Suspected glitch at sample " << samples_per_bit << endl;
  }

  // If this edge is within range to be the central edge in a 1...
  // TODO: make tollerance a setting
  if ((edgeDelta >= (samples_per_transition * 0.75)) &&
      (edgeDelta <= (samples_per_transition * 1.25))) {
    data = 1;

    // Need to advance to next edge to get to the end of the digit
    mSerial->AdvanceToNextEdge();
    secondEdgeSampleNumber = mSerial->GetSampleNumber();
  }

  U64 midpoint = ((secondEdgeSampleNumber - firstEdgeSampleNumber) / 2) + firstEdgeSampleNumber;
  mResults->AddMarker(midpoint,
                      data ? AnalyzerResults::MarkerType::One : AnalyzerResults::MarkerType::Zero,
                      mSettings->mInputChannel);

  return data;
}

void USBPDAnalyzer::DetectPreamble() {
  // USB-PD specification says that we need to be tollerant to losing the first edge of the
  // preamble. Since the first bit of the preamble is always 0, if we lost that edge, then the next
  // edge we see would be the starting edge for the 1 Therefore, we could see two possible
  // bitstreams: 0101010101... repeated for a total of 64 bits 10101010... repreated for a total of
  // 63 bits Since the second is just a subset of the first, we will just look for the second
  // pattern to find the preamble

  bool expected = true;  // Always looking to start the preamble on a '1' bit
  const int expectedPreambleBits = 63;
  int preambleBits = 0;

  U64 startOfPreamble = mSerial->GetSampleNumber();

  while (preambleBits < expectedPreambleBits) {
    bool bit = ReadBiphaseMarkCodeBit();

    // Reset state if we didn't get the expected bit transition
    if (bit != expected) {
      expected = true;   // Always looking to start the preamble on a '1' bit
      preambleBits = 0;  // reset number of bits found
      startOfPreamble =
          mSerial->GetSampleNumber();  // Reset where we think the preamble could start
    } else {
      preambleBits++;
      expected = !expected;
    }
  }

  // We found a preamble!
  U64 endOfPreamble = mSerial->GetSampleNumber();

  // we have a byte to save.
  Frame frame;
  frame.mData1 = 1;
  frame.mFlags = 0;
  frame.mType = FRAME_TYPE_PREAMBLE;
  frame.mStartingSampleInclusive = startOfPreamble;
  frame.mEndingSampleInclusive = endOfPreamble;
  mResults->AddFrame(frame);
}

uint8_t USBPDAnalyzer::ReadFiveBit() {
  uint8_t result = 0;

  // cout << "Reading 5 bits: ";

  for (int i = 0; i < 5; i++) {
    bool bit = ReadBiphaseMarkCodeBit();

    // cout << (bit ? 1 : 0) << ", ";

    // Bits are read LSB -> MSB off the wire
    result |= ((bit ? 0x1 : 0x0) << i);
  }

  // cout << endl;

  return result;
}

/**
 * @brief Use the fiveToFourBitLUT to convert a five-bit input number into a 4-bit output number
 *
 * @param fiveBit
 * @return uint8_t
 */
uint8_t USBPDAnalyzer::ConvertFiveBitToFourBit(uint8_t fiveBit) {
  if (fiveToFourBitLUT.count(fiveBit & 0x1F) == 0) {
    cout << "Unexpceted 5-bit pattern: 0x" << std::hex << (int)fiveBit << endl;
    return 0xFF;
  }

  return fiveToFourBitLUT[fiveBit & 0x1F];
}

/**
 * @brief Read 10 Biphase Mark Coded bits from the stream, decode the 4-to-5-bit encoding, and
 * return the decoded byte.
 *
 * @return uint8_t
 */
uint8_t USBPDAnalyzer::ReadDecodedByte(bool addFrame) {
  U64 startOfByte = mSerial->GetSampleNumber();

  uint8_t fiveBit = ReadFiveBit();
  uint8_t lsbNibble = ConvertFiveBitToFourBit(fiveBit);

  fiveBit = ReadFiveBit();
  uint8_t msbNibble = ConvertFiveBitToFourBit(fiveBit);

  uint8_t data = (((msbNibble << 4) & 0xF0) | (lsbNibble & 0xF));

  U64 endOfByte = mSerial->GetSampleNumber();

  if (addFrame) {
    // we have a byte to save.
    // TODO: support detecting errors in the 5-bit pattern (ConvertFiveBitToFourBit() returns 255)
    // and add a flag so we can put an error in the frame text
    Frame frame;
    frame.mData1 = data;
    frame.mFlags = 0;
    frame.mType = FRAME_TYPE_BYTE;
    frame.mStartingSampleInclusive = startOfByte;
    frame.mEndingSampleInclusive = endOfByte;
    mResults->AddFrame(frame);
  }

  return data;
}

/**
 * @brief Read 40 Biphase Mark Coded bits from the stream, decode the 4-to-5-bit encoding, and
 * return the decoded word.
 *
 * @return uint8_t
 */
uint32_t USBPDAnalyzer::ReadDataObject(uint32_t* currentCrc, bool addFrame) {
  U64 startOfDataObject = mSerial->GetSampleNumber();

  uint8_t byte0 = ReadDecodedByte(false);
  uint8_t byte1 = ReadDecodedByte(false);
  uint8_t byte2 = ReadDecodedByte(false);
  uint8_t byte3 = ReadDecodedByte(false);

  U64 endOfDataObject = mSerial->GetSampleNumber();

  uint32_t dataObject = (byte3 << 24) | (byte2 << 16) | (byte1 << 8) | byte0;

  uint32_t remainder =
      crc32(*currentCrc, (const uint8_t*)&dataObject, sizeof(uint32_t), usbCrcPolynomial);
  *currentCrc = remainder;

  if (addFrame) {
    // we have a byte to save.
    // TODO: support detecting errors in the 5-bit pattern (ConvertFiveBitToFourBit() returns 255)
    // and add a flag so we can put an error in the frame text
    Frame frame;
    frame.mData1 = dataObject;
    frame.mFlags = 0;
    frame.mType = FRAME_TYPE_GENERIC_DATA_OBJECT;
    frame.mStartingSampleInclusive = startOfDataObject;
    frame.mEndingSampleInclusive = endOfDataObject;
    mResults->AddFrame(frame);
  }

  return dataObject;
}

bool USBPDAnalyzer::DetectSOP(SOPType* sop) {
  uint8_t kcode[numKcodeInSOP] = {0};

  U64 startOfSop = mSerial->GetSampleNumber();

  // cout << "Reading bitstream: ";
  for (int i = 0; i < numKcodeInSOP; i++) {
    kcode[i] = ReadFiveBit();
    // cout << (int)kcode[i] << ", ";
  }
  // cout << endl;

  U64 endOfSop = mSerial->GetSampleNumber();

  SOPType detectedSop = NUM_SOP_TYPE;

  for (int i = 0; i < NUM_SOP_TYPE; i++) {
    // Which KCode should we detect for this SOP type?
    const KCODEType* kcodesForSop = sop_map[i];

    // cout << "Looking for KCODE sequence: ";
    // for (int k = 0; k < numKcodeInSOP; k++) {
    //   cout << kcodesForSop[k] << ", ";
    // }
    // cout << endl;

    // How many KCodes matched? If we find 3, we can proceed
    int kcodesFound = 0;
    for (int k = 0; k < numKcodeInSOP; k++) {
      // Which KCode are we currently looking for in this SOP sequence?
      KCODEType currentKcode = kcodesForSop[k];

      // What is the actual 5-bit value for that KCode?
      uint8_t kcodeValue = kcode_map[currentKcode];

      if (kcode[k] == kcodeValue) {
        kcodesFound++;
      }
    }

    // Gottem
    if (kcodesFound >= 3) {
      detectedSop = (SOPType)i;
      break;
    }
  }

  // we have a byte to save.
  Frame frame;
  frame.mData1 = 1;
  frame.mFlags = 0;

  switch (detectedSop) {
    case SOPType_SOP:
      frame.mType = FRAME_TYPE_SOP;
      break;

    case SOPType_SOP_PRIME:
      frame.mType = FRAME_TYPE_SOP_PRIME;
      break;

    case SOPType_SOP_DOUBLE_PRIME:
      frame.mType = FRAME_TYPE_SOP_DOUBLE_PRIME;
      break;

    case SOPType_SOP_PRIME_DEBUG:
      frame.mType = FRAME_TYPE_SOP_PRIME_DEBUG;
      break;

    case SOPType_SOP_DOUBLE_PRIME_DEBUG:
      frame.mType = FRAME_TYPE_SOP_DOUBLE_PRIME_DEBUG;
      break;

    default:
      frame.mType = FRAME_TYPE_SOP_ERROR;
      break;
  }

  *sop = detectedSop;

  frame.mStartingSampleInclusive = startOfSop;
  frame.mEndingSampleInclusive = endOfSop;
  mResults->AddFrame(frame);

  return (detectedSop != NUM_SOP_TYPE);
}

bool USBPDAnalyzer::DetectHeader(SOPType sop,
                                 uint32_t* currentCrc,
                                 uint8_t* dataObjects,
                                 DataMessageTypes* dataMsgType) {
  U64 startOfHeader = mSerial->GetSampleNumber();

  uint8_t lsb = ReadDecodedByte();
  uint8_t msb = ReadDecodedByte();

  U64 endOfHeader = mSerial->GetSampleNumber();

  uint16_t header = (msb << 8) | (lsb);

  *dataObjects = ((header & 0x7000) >> 12);  // Bits 14..12 == Number of Data Objects

  if (*dataObjects > 0) {
    *dataMsgType = (DataMessageTypes)((header & 0xF));  // Bits 3..0 == Message Type
  } else {
    *dataMsgType = NUM_DATA_MESSAGE;
  }

  Frame frame;
  frame.mData1 = header;
  frame.mData2 = sop;
  frame.mFlags = 0;
  frame.mType = FRAME_TYPE_HEADER;
  frame.mStartingSampleInclusive = startOfHeader;
  frame.mEndingSampleInclusive = endOfHeader;
  mResults->AddFrame(frame);

  uint32_t remainder =
      crc32(*currentCrc, (const uint8_t*)&header, sizeof(uint16_t), usbCrcPolynomial);

  *currentCrc = remainder;

  return true;
}

bool USBPDAnalyzer::DetectCRC32(uint32_t* currentCrc) {
  U64 startOfCrc = mSerial->GetSampleNumber();

  uint32_t byte0 = ReadDecodedByte();
  uint32_t byte1 = ReadDecodedByte();
  uint32_t byte2 = ReadDecodedByte();
  uint32_t byte3 = ReadDecodedByte();

  U64 endOfCrc = mSerial->GetSampleNumber();

  uint32_t crcVal = (byte3 << 24) | (byte2 << 16) | (byte1 << 8) | (byte0);

  Frame frame;
  frame.mData1 = crcVal;
  frame.mData2 = *currentCrc;
  frame.mFlags = 0;
  frame.mType = FRAME_TYPE_CRC32;
  frame.mStartingSampleInclusive = startOfCrc;
  frame.mEndingSampleInclusive = endOfCrc;
  mResults->AddFrame(frame);

  return true;
}

bool USBPDAnalyzer::DetectEOP() {
  U64 startOfEop = mSerial->GetSampleNumber();

  uint8_t kcode = ReadFiveBit();

  U64 endOfEop = mSerial->GetSampleNumber();

  Frame frame;
  frame.mData1 = (kcode == kcode_map[KCODEType_EOP]);
  frame.mData2 = 0;
  frame.mFlags = 0;
  frame.mType = FRAME_TYPE_EOP;
  frame.mStartingSampleInclusive = startOfEop;
  frame.mEndingSampleInclusive = endOfEop;
  mResults->AddFrame(frame);

  return true;
}

void USBPDAnalyzer::ReadSourceCapabilities(uint32_t* currentCrc, uint8_t numDataObjects) {
  latestSourceCapabilities.clear();

  for (int i = 0; i < numDataObjects; i++) {
    U64 startOfSourceCapability = mSerial->GetSampleNumber();
    uint32_t pdo = ReadDataObject(currentCrc, false /* don't add a frame */);
    U64 endOfSourceCapability = mSerial->GetSampleNumber();

    Frame frame;
    frame.mData1 = pdo;
    frame.mData2 = 0;
    frame.mFlags = 0;
    frame.mType = FRAME_TYPE_SOURCE_POWER_DATA_OBJECT;
    frame.mStartingSampleInclusive = startOfSourceCapability;
    frame.mEndingSampleInclusive = endOfSourceCapability;
    mResults->AddFrame(frame);

    latestSourceCapabilities.emplace_back(pdo);
  }
}

void USBPDAnalyzer::ReadRequest(uint32_t* currentCrc) {
  U64 startOfRequest = mSerial->GetSampleNumber();
  uint32_t request = ReadDataObject(currentCrc, false /* don't add a frame */);
  U64 endOfRequest = mSerial->GetSampleNumber();

  Frame frame;
  frame.mData1 = request;

  // Which PDO are we referring to from the latestPdo vector?
  // Note: this value starts at 1!! 0 is invalid per the USB-PD spec,
  // so a value of 1 indicates the first entry in the latestPdo vector
  uint8_t objectPosition = EXTRACT_BIT_RANGE(request, 31, 28);

  if (latestSourceCapabilities.size() > (objectPosition - 1)) {
    USBPDMessages::SourcePDO& referencedPdo = latestSourceCapabilities[objectPosition - 1];
    frame.mData2 = referencedPdo.raw;
  } else {
    // Don't have a SourcePDO to reference...
    frame.mData2 = 0xFFFFFFFFFFFFFFFF;
  }

  frame.mFlags = 0;
  frame.mType = FRAME_TYPE_REQUEST_DATA_OBJECT;
  frame.mStartingSampleInclusive = startOfRequest;
  frame.mEndingSampleInclusive = endOfRequest;
  mResults->AddFrame(frame);
}

/**
 * @brief Read the VDO payloads for an ACK'd "DiscoverIdentity" message.
 *
 * @param currentCrc the current payload CRC. this will be updated ad more Data Objects are read
 * from the bus.
 * @param numDataObjects the number of data objects _remaining_ to be read from the DiscoverIdentity
 * message, without including the VDM Header
 * @return uint8_t the number of data objects remaining to be read from the bus. 0 on success,
 * positive values indicate that the DiscoverIdentify payload could not be processed
 */
uint8_t USBPDAnalyzer::ReadDiscoverIdentity(uint32_t* currentCrc, uint8_t numDataObjects) {
  if (numDataObjects < 3) {
    cout << "Invalid DiscoverIdentity command, must have at least 3 data objects, got " << std::dec << numDataObjects << endl;
    return numDataObjects;
  }

  // Read ID Header VDO

  // Read Cert Stat VDO

  // Read Product VDO

  // Read 0-3 Product Type VDOs

  return 0;
}

/**
 * @brief Read a Vendor Defined Message once one has been itentified by the PD Message Header
 *
 * @param currentCrc the current payload CRC. this will be updated ad more Data Objects are read
 * from the bus.
 * @param numDataObjects the number of Data Objects identified in the PD Message header
 */
void USBPDAnalyzer::ReadVendorDefinedMessage(uint32_t* currentCrc, uint8_t numDataObjects) {
  U64 startOfVdmHeader = mSerial->GetSampleNumber();
  uint32_t vdmHeaderData = ReadDataObject(currentCrc, false /* don't add a frame */);
  U64 endOfVdmHeader = mSerial->GetSampleNumber();

  Frame frame;
  frame.mData1 = vdmHeaderData;
  frame.mData2 = 0;
  frame.mFlags = 0;
  frame.mType = FRAME_TYPE_VDM_HEADER;
  frame.mStartingSampleInclusive = startOfVdmHeader;
  frame.mEndingSampleInclusive = endOfVdmHeader;
  mResults->AddFrame(frame);

  USBPDMessages::VDMHeader vdmHeader(vdmHeaderData);

  for (int i = 0; i < (numDataObjects - 1); i++) {
    uint32_t vdo = ReadDataObject(currentCrc, true /* add a frame */);
  }
}

void USBPDAnalyzer::DetectUSBPDTransaction() {
  while (true) {
    // This function will consume edges until we find a Preamble
    DetectPreamble();

    SOPType sop;

    if (!DetectSOP(&sop)) {
      // Failed to detect a SOP after the preamble. Return to searching for a preamble
      continue;
    }

    uint32_t crc32 = 0x00000000;
    uint8_t numDataObjects = 0;
    DataMessageTypes dataMessageType = NUM_DATA_MESSAGE;

    if (!DetectHeader(sop, &crc32, &numDataObjects, &dataMessageType)) {
      // Failed to detect a header after the SOP. Return to searching for a preamble
      continue;
    }

    // TODO: if numDataObjects is still 0, we can't process a data message (since we need at least
    // one data object)

    switch (dataMessageType) {
      case DataMessage_Source_Capabilities: {
        ReadSourceCapabilities(&crc32, numDataObjects);
      } break;

      case DataMessage_Request: {
        ReadRequest(&crc32);
      } break;

      case DataMessage_Vendor_Defined: {
        ReadVendorDefinedMessage(&crc32, numDataObjects);
      } break;

      default: {
        for (int i = 0; i < numDataObjects; i++) {
          ReadDataObject(&crc32);
        }
      } break;
    }

    if (!DetectCRC32(&crc32)) {
      // Failed to detect a CRC32 after the header / payload. Return to searching for a preamble
      continue;
    }

    if (!DetectEOP()) {
      // Failed to detect a SOP after the preamble. Return to searching for a preamble
      continue;
    }

    // Transaciton complete
    break;
  }

  // PD Spec says that we end each frame with an edge edge... skip past this
  // to cleanup our next set of detections
  mSerial->AdvanceToNextEdge();
}

void USBPDAnalyzer::WorkerThread() {
  mSampleRateHz = GetSampleRate();

  mSerial = GetAnalyzerChannelData(mSettings->mInputChannel);

  // Biphase mark coding always starts on a bit-transition
  // All future functions will expect to start on an edge transition, so go there now
  mSerial->AdvanceToNextEdge();

  U32 samples_per_bit = mSampleRateHz / mSettings->mBitRate;
  U32 samples_per_transition =
      mSampleRateHz / (mSettings->mBitRate * 2);  // Two transitions per bit

  U32 samples_to_first_center_of_first_data_bit =
      U32(1.5 * double(mSampleRateHz) / double(mSettings->mBitRate));

  for (;;) {
    DetectUSBPDTransaction();

    /*
    U8 data = 0;
    U8 mask = 1 << 7;

    // Sample number for the first edge
    U64 firstEdgeSampleNumber = mSerial->GetSampleNumber();

    mSerial->AdvanceToNextEdge();

    // Sample number for the second edge
    U64 secondEdgeSampleNumber = mSerial->GetSampleNumber();

    U64 edgeDelta = (secondEdgeSampleNumber - firstEdgeSampleNumber);

    // If this edge is within range to be the central edge in a 1...
    // TODO: make tollerance a setting
    if ((edgeDelta >= (samples_per_transition * 0.75)) &&
        (edgeDelta <= (samples_per_transition * 1.25))) {
        data = 1;

        // Need to advance to next edge to get to the end of the digit
        mSerial->AdvanceToNextEdge();
        secondEdgeSampleNumber = mSerial->GetSampleNumber();
    }

    // we have a byte to save.
    Frame frame;
    frame.mData1 = data;
    frame.mFlags = 0;
    frame.mStartingSampleInclusive = firstEdgeSampleNumber;
    frame.mEndingSampleInclusive = secondEdgeSampleNumber;
    mResults->AddFrame(frame);

    U64 midpoint = ((secondEdgeSampleNumber - firstEdgeSampleNumber) / 2) + firstEdgeSampleNumber;
    mResults->AddMarker(midpoint, data ? AnalyzerResults::MarkerType::One :
    AnalyzerResults::MarkerType::Zero, mSettings->mInputChannel);
    */

    mResults->CommitResults();
    ReportProgress(mSerial->GetSampleNumber());
  }
}

bool USBPDAnalyzer::NeedsRerun() { return false; }

U32 USBPDAnalyzer::GenerateSimulationData(U64 minimum_sample_index,
                                          U32 device_sample_rate,
                                          SimulationChannelDescriptor** simulation_channels) {
  if (mSimulationInitilized == false) {
    mSimulationDataGenerator.Initialize(GetSimulationSampleRate(), mSettings.get());
    mSimulationInitilized = true;
  }

  return mSimulationDataGenerator.GenerateSimulationData(minimum_sample_index,
                                                         device_sample_rate,
                                                         simulation_channels);
}

U32 USBPDAnalyzer::GetMinimumSampleRateHz() { return mSettings->mBitRate * 4; }

const char* USBPDAnalyzer::GetAnalyzerName() const { return "USB Power Delivery (CC)"; }

const char* GetAnalyzerName() { return "USB Power Delivery (CC)"; }

Analyzer* CreateAnalyzer() { return new USBPDAnalyzer(); }

void DestroyAnalyzer(Analyzer* analyzer) { delete analyzer; }
