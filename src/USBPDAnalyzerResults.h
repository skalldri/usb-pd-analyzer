#ifndef USBPD_ANALYZER_RESULTS
#define USBPD_ANALYZER_RESULTS

#include <AnalyzerResults.h>

class USBPDAnalyzer;
class USBPDAnalyzerSettings;

class USBPDAnalyzerResults : public AnalyzerResults {
 public:
  USBPDAnalyzerResults(USBPDAnalyzer* analyzer, USBPDAnalyzerSettings* settings);
  virtual ~USBPDAnalyzerResults();

  virtual void GenerateBubbleText(U64 frame_index, Channel& channel, DisplayBase display_base);
  virtual void GenerateExportFile(const char* file,
                                  DisplayBase display_base,
                                  U32 export_type_user_id);

  virtual void GenerateFrameTabularText(U64 frame_index, DisplayBase display_base);
  virtual void GeneratePacketTabularText(U64 packet_id, DisplayBase display_base);
  virtual void GenerateTransactionTabularText(U64 transaction_id, DisplayBase display_base);

 protected:  // functions
 protected:  // vars
  USBPDAnalyzerSettings* mSettings;
  USBPDAnalyzer* mAnalyzer;
};

#endif  // USBPD_ANALYZER_RESULTS
