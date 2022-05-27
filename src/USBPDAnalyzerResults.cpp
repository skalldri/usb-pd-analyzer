#include "USBPDAnalyzerResults.h"

#include <AnalyzerHelpers.h>

#include <fstream>
#include <iostream>

#include "USBPDAnalyzer.h"
#include "USBPDAnalyzerSettings.h"

USBPDAnalyzerResults::USBPDAnalyzerResults(USBPDAnalyzer* analyzer, USBPDAnalyzerSettings* settings)
    : AnalyzerResults(),
      mSettings(settings),
      mAnalyzer(analyzer) {}

USBPDAnalyzerResults::~USBPDAnalyzerResults() {}

void USBPDAnalyzerResults::GenerateBubbleText(U64 frame_index,
                                              Channel& channel,
                                              DisplayBase display_base) {
  ClearResultStrings();
  Frame frame = GetFrame(frame_index);

  char number_str[128];
  AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 8, number_str, 128);

  switch((FrameType)frame.mType) {
    case FRAME_TYPE_PREAMBLE:
      AddResultString("PREAMBLE");
      break;

    case FRAME_TYPE_SOP:
      AddResultString("SOP");
      break;

    case FRAME_TYPE_SOP_PRIME:
      AddResultString("SOP'");
      break;

    case FRAME_TYPE_SOP_DOUBLE_PRIME:
      AddResultString("SOP\"");
      break;

    case FRAME_TYPE_SOP_PRIME_DEBUG:
      AddResultString("SOP' Debug");
      break;

    case FRAME_TYPE_SOP_DOUBLE_PRIME_DEBUG:
      AddResultString("SOP\" Debug");
      break;

    case FRAME_TYPE_SOP_ERROR:
      AddResultString("!!! SOP ERROR !!!");
      break;

    case FRAME_TYPE_HEADER:
      AddResultString("Header");
      break;
    
    case NUM_FRAME_TYPE:
    default:
      AddResultString(number_str);
      break;

  }
}

void USBPDAnalyzerResults::GenerateExportFile(const char* file,
                                              DisplayBase display_base,
                                              U32 export_type_user_id) {
  std::ofstream file_stream(file, std::ios::out);

  U64 trigger_sample = mAnalyzer->GetTriggerSample();
  U32 sample_rate = mAnalyzer->GetSampleRate();

  file_stream << "Time [s],Value" << std::endl;

  U64 num_frames = GetNumFrames();
  for (U32 i = 0; i < num_frames; i++) {
    Frame frame = GetFrame(i);

    char time_str[128];
    AnalyzerHelpers::GetTimeString(frame.mStartingSampleInclusive,
                                   trigger_sample,
                                   sample_rate,
                                   time_str,
                                   128);

    char number_str[128];
    AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 8, number_str, 128);

    file_stream << time_str << "," << number_str << std::endl;

    if (UpdateExportProgressAndCheckForCancel(i, num_frames) == true) {
      file_stream.close();
      return;
    }
  }

  file_stream.close();
}

void USBPDAnalyzerResults::GenerateFrameTabularText(U64 frame_index, DisplayBase display_base) {
#ifdef SUPPORTS_PROTOCOL_SEARCH
  Frame frame = GetFrame(frame_index);
  ClearTabularText();

  char number_str[128];
  AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 8, number_str, 128);
  AddTabularText(number_str);
#endif
}

void USBPDAnalyzerResults::GeneratePacketTabularText(U64 packet_id, DisplayBase display_base) {
  // not supported
}

void USBPDAnalyzerResults::GenerateTransactionTabularText(U64 transaction_id,
                                                          DisplayBase display_base) {
  // not supported
}