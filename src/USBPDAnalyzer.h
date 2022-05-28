#ifndef USBPD_ANALYZER_H
#define USBPD_ANALYZER_H

#include <Analyzer.h>

#include <map>

#include "USBPDAnalyzerResults.h"
#include "USBPDSimulationDataGenerator.h"
#include "USBPDTypes.h"

class USBPDAnalyzerSettings;
class ANALYZER_EXPORT USBPDAnalyzer : public Analyzer2 {
 public:
  USBPDAnalyzer();
  virtual ~USBPDAnalyzer();

  virtual void SetupResults();
  virtual void WorkerThread();

  virtual U32 GenerateSimulationData(U64 newest_sample_requested,
                                     U32 sample_rate,
                                     SimulationChannelDescriptor** simulation_channels);
  virtual U32 GetMinimumSampleRateHz();

  virtual const char* GetAnalyzerName() const;
  virtual bool NeedsRerun();

 protected:  // vars
  std::auto_ptr<USBPDAnalyzerSettings> mSettings;
  std::auto_ptr<USBPDAnalyzerResults> mResults;
  AnalyzerChannelData* mSerial;

  USBPDSimulationDataGenerator mSimulationDataGenerator;
  bool mSimulationInitilized;

  // Serial analysis vars:
  U32 mSampleRateHz;
  U32 mStartOfStopBitOffset;
  U32 mEndOfStopBitOffset;

  std::map<uint8_t, uint8_t> fiveToFourBitLUT;

 protected:
  void DetectPreamble();
  bool DetectSOP();
  bool DetectHeader(uint32_t* currentCrc, uint8_t* dataObjects);

  bool DetectEOP();
  bool DetectCRC32(uint32_t* currentCrc);

  uint8_t ReadFiveBit();
  uint8_t ConvertFiveBitToFourBit(uint8_t fiveBit);

  uint8_t ReadDecodedByte(bool addFrame = false);

  uint32_t ReadDataObject(uint32_t* currentCrc, bool addFrame = true);

  bool ReadBiphaseMarkCodeBit();
  void DetectUSBPDTransaction();
};

extern "C" ANALYZER_EXPORT const char* __cdecl GetAnalyzerName();
extern "C" ANALYZER_EXPORT Analyzer* __cdecl CreateAnalyzer();
extern "C" ANALYZER_EXPORT void __cdecl DestroyAnalyzer(Analyzer* analyzer);

#endif  // USBPD_ANALYZER_H
