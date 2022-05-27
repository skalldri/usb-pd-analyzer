#include "USBPDAnalyzer.h"

#include <AnalyzerChannelData.h>

#include "USBPDAnalyzerSettings.h"

#include <iostream>

using namespace std;

USBPDAnalyzer::USBPDAnalyzer()
    : Analyzer2(),
      mSettings(new USBPDAnalyzerSettings()),
      mSimulationInitilized(false) {
  SetAnalyzerSettings(mSettings.get());
}

USBPDAnalyzer::~USBPDAnalyzer() { KillThread(); }

void USBPDAnalyzer::SetupResults() {
  mResults.reset(new USBPDAnalyzerResults(this, mSettings.get()));
  SetAnalyzerResults(mResults.get());
  mResults->AddChannelBubblesWillAppearOn(mSettings->mInputChannel);
}

/*
void USBPDAnalyzer::WorkerThread() {
  mSampleRateHz = GetSampleRate();

  mSerial = GetAnalyzerChannelData(mSettings->mInputChannel);

  if (mSerial->GetBitState() == BIT_LOW) mSerial->AdvanceToNextEdge();

  U32 samples_per_bit = mSampleRateHz / mSettings->mBitRate;
  U32 samples_to_first_center_of_first_data_bit =
      U32(1.5 * double(mSampleRateHz) / double(mSettings->mBitRate));

  for (;;) {
    U8 data = 0;
    U8 mask = 1 << 7;

    mSerial->AdvanceToNextEdge();  // falling edge -- beginning of the start bit

    U64 starting_sample = mSerial->GetSampleNumber();

    mSerial->Advance(samples_to_first_center_of_first_data_bit);

    for (U32 i = 0; i < 8; i++) {
      // let's put a dot exactly where we sample this bit:
      mResults->AddMarker(mSerial->GetSampleNumber(),
                          AnalyzerResults::Dot,
                          mSettings->mInputChannel);

      if (mSerial->GetBitState() == BIT_HIGH) data |= mask;

      mSerial->Advance(samples_per_bit);

      mask = mask >> 1;
    }

    // we have a byte to save.
    Frame frame;
    frame.mData1 = data;
    frame.mFlags = 0;
    frame.mStartingSampleInclusive = starting_sample;
    frame.mEndingSampleInclusive = mSerial->GetSampleNumber();

    mResults->AddFrame(frame);
    mResults->CommitResults();
    ReportProgress(frame.mEndingSampleInclusive);
  }
}
*/

// Needs to start on an edge!
bool USBPDAnalyzer::ReadBiphaseMarkCodeBit() {
  U32 samples_per_bit = mSampleRateHz / mSettings->mBitRate;
  U32 samples_per_transition = mSampleRateHz / (mSettings->mBitRate * 2); // Two transitions per bit

  U8 data = 0;

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

  U64 midpoint = ((secondEdgeSampleNumber - firstEdgeSampleNumber) / 2) + firstEdgeSampleNumber;
  mResults->AddMarker(midpoint, data ? AnalyzerResults::MarkerType::One : AnalyzerResults::MarkerType::Zero, mSettings->mInputChannel);

  return data;
}

void USBPDAnalyzer::DetectPreamble() {
  // USB-PD specification says that we need to be tollerant to losing the first edge of the preamble.
  // Since the first bit of the preamble is always 0, if we lost that edge, then
  // the next edge we see would be the starting edge for the 1
  // Therefore, we could see two possible bitstreams:
  // 0101010101... repeated for a total of 64 bits
  // 10101010... repreated for a total of 63 bits
  // Since the second is just a subset of the first, we will just look for the second pattern to find the preamble

  bool expected = true; // Always looking to start the preamble on a '1' bit
  const int expectedPreambleBits = 63;
  int preambleBits = 0;

  U64 startOfPreamble = mSerial->GetSampleNumber();

  while (preambleBits < expectedPreambleBits) {
    bool bit = ReadBiphaseMarkCodeBit();

    // Reset state if we didn't get the expected bit transition
    if (bit != expected) {
      expected = true; // Always looking to start the preamble on a '1' bit
      preambleBits = 0; // reset number of bits found
      startOfPreamble = mSerial->GetSampleNumber(); // Reset where we think the preamble could start
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

uint8_t USBPDAnalyzer::Read5Bit() {
  uint8_t result = 0;

  cout << "Reading 5 bits: ";

  for (int i = 0; i < 5; i++) {
    bool bit = ReadBiphaseMarkCodeBit();

    cout << (bit ? 1 : 0) << ", ";

    // Bits are read LSB -> MSB off the wire
    result |= ((bit ? 0x1 : 0x0) << i);
  }

  cout << endl;

  return result;
}

bool USBPDAnalyzer::DetectSOP() {
  uint8_t kcode[numKcodeInSOP] = {0};

  U64 startOfSop = mSerial->GetSampleNumber();


  cout << "Reading bitstream: ";
  for(int i = 0; i < numKcodeInSOP; i++) {
    kcode[i] = Read5Bit();
    cout << (int) kcode[i] << ", ";
  }
  cout << endl;

  U64 endOfSop = mSerial->GetSampleNumber();

  SOPTypes detectedSop = NUM_SOP_TYPE;

  for (int i = 0; i < NUM_SOP_TYPE; i++) {
    // Which KCode should we detect for this SOP type?
    const KCODE* kcodesForSop = sop_map[i];

    cout << "Looking for KCODE sequence: ";
    for (int k = 0; k < numKcodeInSOP; k++) {
        cout << kcodesForSop[k] << ", ";
    }
    cout << endl;

    // How many KCodes matched? If we find 3, we can proceed
    int kcodesFound = 0;
    for (int k = 0; k < numKcodeInSOP; k++) {
      // Which KCode are we currently looking for in this SOP sequence?
      KCODE currentKcode = kcodesForSop[k];

      // What is the actual 5-bit value for that KCode?
      uint8_t kcodeValue = kcode_map[currentKcode];

      if (kcode[k] == kcodeValue) {
        kcodesFound++;
      }
    }

    // Gottem
    if (kcodesFound >= 3) {
      detectedSop = (SOPTypes)i;
      break;
    }
  }

  // we have a byte to save.
  Frame frame;
  frame.mData1 = 1;
  frame.mFlags = 0;

  switch(detectedSop){
    case SOP:
      frame.mType = FRAME_TYPE_SOP;
      break;

    case SOP_PRIME:
      frame.mType = FRAME_TYPE_SOP_PRIME;
      break;

    case SOP_DOUBLE_PRIME:
      frame.mType = FRAME_TYPE_SOP_DOUBLE_PRIME;
      break;

    case SOP_PRIME_DEBUG:
      frame.mType = FRAME_TYPE_SOP_PRIME_DEBUG;
      break;

    case SOP_DOUBLE_PRIME_DEBUG:
      frame.mType = FRAME_TYPE_SOP_DOUBLE_PRIME_DEBUG;
      break;
    
    case NUM_SOP_TYPE:
    default:
      frame.mType = FRAME_TYPE_SOP_ERROR;
      break;
  }
  
  frame.mStartingSampleInclusive = startOfSop;
  frame.mEndingSampleInclusive = endOfSop;
  mResults->AddFrame(frame);

  return (detectedSop != NUM_SOP_TYPE);
}

void USBPDAnalyzer::DetectUSBPDTransaction() {
  while (true) {
    // This function will consume edges until we find a Preamble
    DetectPreamble();

    if (!DetectSOP()) {
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
  U32 samples_per_transition = mSampleRateHz / (mSettings->mBitRate * 2); // Two transitions per bit

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
    mResults->AddMarker(midpoint, data ? AnalyzerResults::MarkerType::One : AnalyzerResults::MarkerType::Zero, mSettings->mInputChannel);
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