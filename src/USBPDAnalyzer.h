#ifndef USBPD_ANALYZER_H
#define USBPD_ANALYZER_H

#include <Analyzer.h>

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

 protected:
  void DetectPreamble();
  bool DetectSOP();
  uint8_t Read5Bit();

  bool ReadBiphaseMarkCodeBit();
  void DetectUSBPDTransaction();

};

extern "C" ANALYZER_EXPORT const char* __cdecl GetAnalyzerName();
extern "C" ANALYZER_EXPORT Analyzer* __cdecl CreateAnalyzer();
extern "C" ANALYZER_EXPORT void __cdecl DestroyAnalyzer(Analyzer* analyzer);

#endif  // USBPD_ANALYZER_H
