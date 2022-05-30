#ifndef USBPD_SIMULATION_DATA_GENERATOR
#define USBPD_SIMULATION_DATA_GENERATOR

#include <SimulationChannelDescriptor.h>

#include <string>

#include "USBPDTypes.h"
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

  void CreateByte(uint8_t byte);

  void CreateFiveBitFromFourBit(uint8_t fourBit);

  void CreateBiphaseMarkCodingBit(bool bit);

  void CreateUSBPDControlMessageTransaction(SOPType sop,
                                            ControlMessageTypes messageType,
                                            PortDataRole dataRole,
                                            PDSpecRevision specRev,
                                            uint8_t portPowerRoleOrCablePlug);

  uint16_t CreateMessageHeader(uint8_t messageType,
                               uint8_t portDataRole,
                               uint8_t specificationRevision,
                               uint8_t portPowerRoleOrCablePlug,
                               uint8_t messageId,
                               uint8_t numOfDataObjects);

  uint16_t CreateControlMessageHeader(ControlMessageTypes type,
                                      PortDataRole role,
                                      PDSpecRevision rev,
                                      uint8_t portPowerRoleOrCablePlug,
                                      uint8_t messageId);

  void CreateSOP(SOPType sop);
  void CreateKCode(KCODEType code);
  uint8_t FourBitToFiveBitEncoder(uint8_t val);
};
#endif  // USBPD_SIMULATION_DATA_GENERATOR