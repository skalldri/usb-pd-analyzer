#include "USBPDSimulationDataGenerator.h"

#include <AnalyzerHelpers.h>

#include "USBPDAnalyzerSettings.h"
#include "USBPDTypes.h"

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
    CreateUSBPDTransaction();
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
  U32 samples_per_transition = mSimulationSampleRateHz / (mSettings->mBitRate * 2); // Two transitions per bit

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

void USBPDSimulationDataGenerator::CreateBiphaseMarkCodingByte(uint8_t byte) {
  for(int i = 0; i < 8; i++) {
    CreateBiphaseMarkCodingBit(byte & 0x1);
    byte >>= 1;
  }
}

void USBPDSimulationDataGenerator::CreatePreamble() {
  // Always start with transmitting a 0
  bool bit = false;

  for(int i = 0; i < 64; i++) {
    CreateBiphaseMarkCodingBit(bit);
    bit = !bit;
  }
}

void USBPDSimulationDataGenerator::CreateSOP() {
  CreateKCode(SYNC_1);
  CreateKCode(SYNC_1);
  CreateKCode(SYNC_1);
  CreateKCode(SYNC_2);
}

void USBPDSimulationDataGenerator::CreateKCode(KCODE code) {

  if (code >= NUM_KCODE) {
    return;
  }

  uint8_t kcode_value = kcode_map[code];
  for(int i = 0; i < numKcodeBits; i++) {
    CreateBiphaseMarkCodingBit(kcode_value & 0x1);
    kcode_value >>= 1;
  }
}

/*
void USBPDSimulationDataGenerator::CreatePreamble() {
  U32 samples_per_bit = mSimulationSampleRateHz / mSettings->mBitRate;

  // we're currenty high
  // let's move forward a little
  mSerialSimulationData.Advance(samples_per_bit);

  // Ensure we start in the high state
  // TODO: simulator settings that allow us to start in either the high or low state: either
  // state is technically valid for Biphase Mark Coding
  mSerialSimulationData.Transition();
}
*/

void USBPDSimulationDataGenerator::CreateUSBPDTransaction() {
  U32 samples_per_bit = mSimulationSampleRateHz / mSettings->mBitRate;

  // Ensure we start in the high state
  // TODO: simulator settings that allow us to start in either the high or low state: either
  // state is technically valid for Biphase Mark Coding
  mSerialSimulationData.TransitionIfNeeded(BIT_HIGH);

  // we're currenty high
  // let's move forward a little
  mSerialSimulationData.Advance(samples_per_bit * 10);

  CreatePreamble();

  CreateSOP();

  // All Frames end with a final edge transition
  mSerialSimulationData.Transition();

  mSerialSimulationData.Advance(samples_per_bit * 10);
}