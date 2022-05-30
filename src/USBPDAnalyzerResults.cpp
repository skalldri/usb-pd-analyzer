#include "USBPDAnalyzerResults.h"

#include <AnalyzerHelpers.h>

#include <fstream>
#include <iostream>

#include "USBPDAnalyzer.h"
#include "USBPDAnalyzerSettings.h"
#include "USBPDMessages.h"

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

  switch ((FrameType)frame.mType) {
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

    case FRAME_TYPE_HEADER: {
      // Header is a 16 bit number that we will fully store within mData1
      // Detected SOPType for this transaction is stored in mData2

      SOPType sop = (SOPType)frame.mData2;
      USBPDMessages::Header header(sop, (uint16_t)frame.mData1);

      char msgIdString[128];
      AnalyzerHelpers::GetNumberString(header.messageId, display_base, 3, msgIdString, 128);

      char result_str[1028];

      if (sop == SOPType_SOP) {
        sprintf(
            result_str,
            "Header (%s), Msg Source (Port Data Role)=%s, Port Power Role=%s, MsgID=%s, Spec "
            "Rev=%s",
            header.numberOfDataObjects == 0 ? ControlMessageNames[header.messageType]
                                            : DataMessageNames[header.messageType],
            (header.portDataRole == PortDataRole_UFP) ? "UFP Port" : "DFP Port",
            (header.portPowerRoleOrCablePlug == PortPowerRole_Source) ? "Source" : "Sink",
            msgIdString,
            header.specRev == PDSpecRevision_1P0
                ? "PD 1.0"
                : (header.specRev == PDSpecRevision_2P0
                       ? "PD 2.0"
                       : (header.specRev == PDSpecRevision_3P0 ? "PD 3.0" : "Unknown PD Spec")));

      } else {
        sprintf(
            result_str,
            "Header (%s), Msg Source (Cable Plug)=%s, MsgID=%s, Spec Rev=%s",
            header.numberOfDataObjects == 0 ? ControlMessageNames[header.messageType]
                                            : DataMessageNames[header.messageType],
            (header.portPowerRoleOrCablePlug == CablePlug_MsgSrcPort) ? "DFP/UFP Port"
                                                                      : "Cable Plug",
            msgIdString,
            header.specRev == PDSpecRevision_1P0
                ? "PD 1.0"
                : (header.specRev == PDSpecRevision_2P0
                       ? "PD 2.0"
                       : (header.specRev == PDSpecRevision_3P0 ? "PD 3.0" : "Unknown PD Spec")));
      }

      AddResultString(result_str);
    } break;

    case FRAME_TYPE_SOURCE_POWER_DATA_OBJECT: {
      // PDO is a 32 bit number stored in mData1
      USBPDMessages::SourcePDO pdo(frame.mData1);
      char result_str[1028];

      switch (pdo.type) {
        case PDOType_FixedSupply: {
          sprintf(result_str,
                  "PDO - Fixed Supply, "
                  "Dual-Role Power Capable=%s, "
                  "Dual-Role Data Capable=%s, "
                  "USB Suspend Supported=%s, "
                  "USB Comms Capable=%s, "
                  "Unconstrained Power=%s, "
                  "Unchunked Extended Msgs Supported=%s, "
                  "EPR Mode Capable=%s, "
                  "Peak Current Mode=%d, "
                  "Voltage=%d mV, "
                  "MaxCurrent=%d mA",
                  pdo.fixedSupplyPdo.dualRolePower ? "Yes" : "No",
                  pdo.fixedSupplyPdo.dualRoleData ? "Yes" : "No",
                  pdo.fixedSupplyPdo.usbSuspendSupported ? "Yes" : "No",
                  pdo.fixedSupplyPdo.usbCommunicationsCapable ? "Yes" : "No",
                  pdo.fixedSupplyPdo.unconstrainedPower ? "Yes" : "No",
                  pdo.fixedSupplyPdo.unchunkedExtendedMessagesSupported ? "Yes" : "No",
                  pdo.fixedSupplyPdo.eprModeCapable ? "Yes" : "No",
                  pdo.fixedSupplyPdo.peakCurrentMode,
                  pdo.fixedSupplyPdo.voltage_mV,
                  pdo.fixedSupplyPdo.maxCurrent_mA);
        } break;

        case PDOType_Battery: {
          sprintf(result_str,
                  "PDO - Battery, "
                  "MaxVoltage=%d mV, "
                  "MinVoltage=%d mV, "
                  "MaxPower=%d mW, ",
                  pdo.batteryPdo.maxVoltage_mV,
                  pdo.batteryPdo.minVoltage_mV,
                  pdo.batteryPdo.maxPower_mW);
        } break;

        case PDOType_VariableSupply: {
          sprintf(result_str,
                  "PDO - Variable Supply, "
                  "MaxVoltage=%d mV, "
                  "MinVoltage=%d mV, "
                  "MaxCurrent=%d mA, ",
                  pdo.variableSupplyPdo.maxVoltage_mV,
                  pdo.variableSupplyPdo.minVoltage_mV,
                  pdo.variableSupplyPdo.maxCurrent_mA);
        } break;

        case PDOType_AugmentedPDO: {
          switch (pdo.augmentedPdo.type) {
            case APDOType_SPRProgrammablePowerSupply: {
              sprintf(result_str,
                      "APDO - SPR Programmable Power Supply, "
                      "PPS Power Limited=%s, "
                      "MaxVoltage=%d mV, "
                      "MinVoltage=%d mV, "
                      "MaxCurrent=%d mA",
                      pdo.augmentedPdo.ppsPdo.ppsPowerLimited ? "Yes" : "No",
                      pdo.augmentedPdo.ppsPdo.maxVoltage_mV,
                      pdo.augmentedPdo.ppsPdo.minVoltage_mV,
                      pdo.augmentedPdo.ppsPdo.maxCurrent_mA);
            } break;

            case APDOType_EPRAdjustableVoltageSupply: {
              sprintf(result_str,
                      "APDO - EPR Adjustable Voltage Supply, "
                      "Peak Current Mode=%d, "
                      "MaxVoltage=%d mV, "
                      "MinVoltage=%d mV, "
                      "PDPPower=%d mW, ",
                      pdo.augmentedPdo.avsPdo.peakCurrentMode,
                      pdo.augmentedPdo.avsPdo.maxVoltage_mV,
                      pdo.augmentedPdo.avsPdo.minVoltage_mV,
                      pdo.augmentedPdo.avsPdo.pdpPower_mW);
            } break;

            default: {
              sprintf(result_str, "!!! APDO - Invalid Type %d !!!", pdo.augmentedPdo.type);
            } break;
          }
        } break;

        default: {
          sprintf(result_str, "!!! PDO - Invalid Type %d !!!", pdo.type);
        } break;
      }

      AddResultString(result_str);
    } break;

    case FRAME_TYPE_REQUEST_DATA_OBJECT: {
      // RDO is passed in via mData1
      // The referenced PDO is passed in via mData2, or MAX_UINT64 if invalid reference

      char result_str[1028];

      if (frame.mData2 == 0xFFFFFFFFFFFFFFFF) {
        sprintf(result_str,
                "!!! RDO References Invalid PDO Index: %d !!!",
                EXTRACT_BIT_RANGE(frame.mData1, 31, 28));
      } else {
        USBPDMessages::SourcePDO pdo(frame.mData2);
        USBPDMessages::Request request(pdo, frame.mData1);

        switch (request.type) {
          case PDOType_FixedSupply: {
            sprintf(result_str,
                    "Request - Fixed Supply (%d mV), "
                    "Give Back Support=%s, "
                    "Capability Mismatch=%s, "
                    "USB Comms Capable=%s, "
                    "No USB Suspend=%s, "
                    "Unchunked Extended Msgs Supported=%s, "
                    "EPR Mode Capable=%s, "
                    "Operating Current=%d mA, "
                    "%s Operating Current=%d mA, ",
                    pdo.fixedSupplyPdo.voltage_mV,
                    request.fixedSupplyRequest.giveBack ? "Yes" : "No",
                    request.fixedSupplyRequest.capabilityMismatch ? "Yes" : "No",
                    request.fixedSupplyRequest.usbCommunicationCapable ? "Yes" : "No",
                    request.fixedSupplyRequest.noUsbSuspend ? "Yes" : "No",
                    request.fixedSupplyRequest.unchunkedExtendedMessagesSupported ? "Yes" : "No",
                    request.fixedSupplyRequest.eprModeCapable ? "Yes" : "No",
                    request.fixedSupplyRequest.operatingCurrent_mA,
                    request.fixedSupplyRequest.giveBack ? "Minimum" : "Maximum",
                    request.fixedSupplyRequest.giveBack
                        ? request.fixedSupplyRequest.minOperatingCurrent_mA
                        : request.fixedSupplyRequest.maxOperatingCurrent_mA);
          } break;

          case PDOType_Battery: {
            sprintf(result_str,
                    "Request - Battery, "
                    "Give Back Support=%s, "
                    "Capability Mismatch=%s, "
                    "USB Comms Capable=%s, "
                    "No USB Suspend=%s, "
                    "Unchunked Extended Msgs Supported=%s, "
                    "EPR Mode Capable=%s, "
                    "Operating Power=%d mW, "
                    "%s Operating Power=%d mW, ",
                    request.batterySupplyRequest.giveBack ? "Yes" : "No",
                    request.batterySupplyRequest.capabilityMismatch ? "Yes" : "No",
                    request.batterySupplyRequest.usbCommunicationCapable ? "Yes" : "No",
                    request.batterySupplyRequest.noUsbSuspend ? "Yes" : "No",
                    request.batterySupplyRequest.unchunkedExtendedMessagesSupported ? "Yes" : "No",
                    request.batterySupplyRequest.eprModeCapable ? "Yes" : "No",
                    request.batterySupplyRequest.operatingPower_mW,
                    request.batterySupplyRequest.giveBack ? "Minimum" : "Maximum",
                    request.batterySupplyRequest.giveBack
                        ? request.batterySupplyRequest.minOperatingPower_mW
                        : request.batterySupplyRequest.maxOperatingPower_mW);
          } break;

          case PDOType_VariableSupply: {
            sprintf(result_str,
                    "Request - Variable Supply, "
                    "Give Back Support=%s, "
                    "Capability Mismatch=%s, "
                    "USB Comms Capable=%s, "
                    "No USB Suspend=%s, "
                    "Unchunked Extended Msgs Supported=%s, "
                    "EPR Mode Capable=%s, "
                    "Operating Current=%d mA, "
                    "%s Operating Current=%d mA",
                    request.variableSupplyRequest.giveBack ? "Yes" : "No",
                    request.variableSupplyRequest.capabilityMismatch ? "Yes" : "No",
                    request.variableSupplyRequest.usbCommunicationCapable ? "Yes" : "No",
                    request.variableSupplyRequest.noUsbSuspend ? "Yes" : "No",
                    request.variableSupplyRequest.unchunkedExtendedMessagesSupported ? "Yes" : "No",
                    request.variableSupplyRequest.eprModeCapable ? "Yes" : "No",
                    request.variableSupplyRequest.operatingCurrent_mA,
                    request.variableSupplyRequest.giveBack ? "Minimum" : "Maximum",
                    request.variableSupplyRequest.giveBack
                        ? request.variableSupplyRequest.minOperatingCurrent_mA
                        : request.variableSupplyRequest.maxOperatingCurrent_mA);
          } break;

          case PDOType_AugmentedPDO: {
            switch (pdo.augmentedPdo.type) {
              case APDOType_SPRProgrammablePowerSupply: {
                sprintf(result_str,
                    "Request - Programmable Power Supply, "
                    "Capability Mismatch=%s, "
                    "USB Comms Capable=%s, "
                    "No USB Suspend=%s, "
                    "Unchunked Extended Msgs Supported=%s, "
                    "EPR Mode Capable=%s, "
                    "Output Voltage=%d mV, "
                    "Operating Current=%d mA",
                    request.ppsRequest.capabilityMismatch ? "Yes" : "No",
                    request.ppsRequest.usbCommunicationCapable ? "Yes" : "No",
                    request.ppsRequest.noUsbSuspend ? "Yes" : "No",
                    request.ppsRequest.unchunkedExtendedMessagesSupported ? "Yes" : "No",
                    request.ppsRequest.eprModeCapable ? "Yes" : "No",
                    request.ppsRequest.outputVoltage_mV,
                    request.ppsRequest.operatingCurrent_mA);
              } break;

              case APDOType_EPRAdjustableVoltageSupply: {
                sprintf(result_str,
                    "Request - Programmable Power Supply, "
                    "Capability Mismatch=%s, "
                    "USB Comms Capable=%s, "
                    "No USB Suspend=%s, "
                    "Unchunked Extended Msgs Supported=%s, "
                    "EPR Mode Capable=%s, "
                    "Output Voltage=%d mV, "
                    "Operating Current=%d mA",
                    request.avsRequest.capabilityMismatch ? "Yes" : "No",
                    request.avsRequest.usbCommunicationCapable ? "Yes" : "No",
                    request.avsRequest.noUsbSuspend ? "Yes" : "No",
                    request.avsRequest.unchunkedExtendedMessagesSupported ? "Yes" : "No",
                    request.avsRequest.eprModeCapable ? "Yes" : "No",
                    request.avsRequest.outputVoltage_mV,
                    request.avsRequest.operatingCurrent_mA);
              } break;

              default: {
                sprintf(result_str, "!!! APDO - Invalid Type %d !!!", pdo.augmentedPdo.type);
              } break;
            }
          } break;

          default: {
            sprintf(result_str, "!!! PDO - Invalid Type %d !!!", pdo.type);
          } break;
        }
      }

      AddResultString(result_str);
    } break;

    case FRAME_TYPE_CRC32: {
      // Received CRC32 is passed in mData1
      // CRC32 of all message bytes up to this point is passed in via mData2
      char calculated[128];
      char received[128];
      AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 32, calculated, 128);
      AnalyzerHelpers::GetNumberString(frame.mData2, display_base, 32, received, 128);
      AddResultString("CRC32: Calculated=", calculated, ", Received=", received);
    } break;

    case FRAME_TYPE_EOP: {
      // mData1 == 1 if the received KCODE == the EOP Kcode
      AddResultString(frame.mData1 ? "EOP" : "EOP ERROR");
    } break;

    case FRAME_TYPE_VDM_HEADER: {
      // VdmHeader is a 32 bit number that we will fully store within mData1

      USBPDMessages::VDMHeader header((uint32_t)frame.mData1);

      char result_str[1028];

      if (header.type == VDMType_Structured) {
        sprintf(
            result_str,
            "VDM (Structured), "
            "VID=%16x, "
            "Version=%s, "
            "Object Position=%d, "
            "Command Type=%s, "
            "Command=%s",
            header.vid,
            StructuredVDMVersionNames[header.structuredData.version],
            header.structuredData.objectPosition,
            StructuredVDMCommandTypeNames[header.structuredData.commandType],
            StructuredVDMCommandNames[header.structuredData.command]);

      } else {
        sprintf(
            result_str,
            "VDM (Unstructured), "
            "VID=%16x, "
            "Data=%16x, ",
            header.vid,
            header.unstructuredData);
      }

      AddResultString(result_str);
    } break;

    case FRAME_TYPE_GENERIC_DATA_OBJECT: {
      // Generic Data Objects are stored in mData1, and are 32 bits
      char dataObject[128];
      AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 32, dataObject, 128);
      AddResultString("DATA=", dataObject);
    } break;

    case FRAME_TYPE_BYTE:
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