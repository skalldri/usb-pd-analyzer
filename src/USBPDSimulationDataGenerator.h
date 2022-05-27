#ifndef USBPD_SIMULATION_DATA_GENERATOR
#define USBPD_SIMULATION_DATA_GENERATOR

#include <SimulationChannelDescriptor.h>

#include "USBPDTypes.h"

#include <string>
class USBPDAnalyzerSettings;

class USBPDSimulationDataGenerator {
 public:
  USBPDSimulationDataGenerator();
  ~USBPDSimulationDataGenerator();

  void Initialize(U32 simulation_sample_rate, USBPDAnalyzerSettings* settings);
  U32 GenerateSimulationData(U64 newest_sample_requested,
                             U32 sample_rate,
                             SimulationChannelDescriptor** simulation_channel);

  // Use 2x the USB-PD CC channel clock rate for Nyquist freq?
  // virtual U32 GetMinimumSampleRateHz();

 protected:
  USBPDAnalyzerSettings* mSettings;
  U32 mSimulationSampleRateHz;
  std::string mSerialText;
  U32 mStringIndex;
  SimulationChannelDescriptor mSerialSimulationData;

 protected:
  void CreateSerialByte();
  void CreatePreamble();
  void CreateBiphaseMarkCodingByte(uint8_t byte);
  void CreateBiphaseMarkCodingBit(bool bit);
  void CreateUSBPDTransaction();
  void CreateSOP();
  void CreateKCode(KCODE code);
};
#endif  // USBPD_SIMULATION_DATA_GENERATOR