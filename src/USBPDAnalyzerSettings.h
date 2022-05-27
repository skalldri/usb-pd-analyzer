#ifndef USBPD_ANALYZER_SETTINGS
#define USBPD_ANALYZER_SETTINGS

#include <AnalyzerSettings.h>
#include <AnalyzerTypes.h>

class USBPDAnalyzerSettings : public AnalyzerSettings {
 public:
  USBPDAnalyzerSettings();
  virtual ~USBPDAnalyzerSettings();

  virtual bool SetSettingsFromInterfaces();
  void UpdateInterfacesFromSettings();
  virtual void LoadSettings(const char* settings);
  virtual const char* SaveSettings();

  Channel mInputChannel;
  U32 mBitRate;

 protected:
  std::auto_ptr<AnalyzerSettingInterfaceChannel> mInputChannelInterface;
  std::auto_ptr<AnalyzerSettingInterfaceInteger> mBitRateInterface;
};

#endif  // USBPD_ANALYZER_SETTINGS
