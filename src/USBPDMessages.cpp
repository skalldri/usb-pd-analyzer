#include "USBPDMessages.h"

using namespace USBPDMessages;

FixedSupplySourcePDO::FixedSupplySourcePDO(uint32_t val) {
  dualRolePower = CHECK_BIT(val, 29);
  usbSuspendSupported = CHECK_BIT(val, 28);
  unconstrainedPower = CHECK_BIT(val, 27);
  usbCommunicationsCapable = CHECK_BIT(val, 26);
  dualRoleData = CHECK_BIT(val, 25);
  unchunkedExtendedMessagesSupported = CHECK_BIT(val, 24);
  eprModeCapable = CHECK_BIT(val, 23);
  peakCurrentMode = EXTRACT_BIT_RANGE(val, 21, 20);
  voltage_mV = EXTRACT_BIT_RANGE(val, 19, 10) * usbPdoMilivoltPerStep;
  maxCurrent_mA = EXTRACT_BIT_RANGE(val, 9, 0) * usbPdoMiliampPerStep;
}

BatterySourcePDO::BatterySourcePDO(uint32_t val) {
  maxVoltage_mV = EXTRACT_BIT_RANGE(val, 29, 20) * usbPdoMilivoltPerStep;
  minVoltage_mV = EXTRACT_BIT_RANGE(val, 19, 10) * usbPdoMilivoltPerStep;
  maxPower_mW = EXTRACT_BIT_RANGE(val, 9, 0) * usbPdoMiliwattPerStep;
}

VariableSupplySourcePDO::VariableSupplySourcePDO(uint32_t val) {
  maxVoltage_mV = EXTRACT_BIT_RANGE(val, 29, 20) * usbPdoMilivoltPerStep;
  minVoltage_mV = EXTRACT_BIT_RANGE(val, 19, 10) * usbPdoMilivoltPerStep;
  maxCurrent_mA = EXTRACT_BIT_RANGE(val, 9, 0) * usbPdoMiliampPerStep;
}

SPRProgrammablePowerSupplySourcePDO::SPRProgrammablePowerSupplySourcePDO(uint32_t val) {
  ppsPowerLimited = CHECK_BIT(val, 27);
  maxVoltage_mV = EXTRACT_BIT_RANGE(val, 24, 17) * usbApdoMilivoltPerStep;
  minVoltage_mV = EXTRACT_BIT_RANGE(val, 15, 8) * usbApdoMilivoltPerStep;
  maxCurrent_mA = EXTRACT_BIT_RANGE(val, 6, 0) * usbApdoMiliampPerStep;
}

EPRAdjustableVoltageSupplySourcePDO::EPRAdjustableVoltageSupplySourcePDO(uint32_t val) {
  peakCurrentMode = EXTRACT_BIT_RANGE(val, 27, 26);
  maxVoltage_mV = EXTRACT_BIT_RANGE(val, 25, 17) * usbApdoMilivoltPerStep;
  minVoltage_mV = EXTRACT_BIT_RANGE(val, 15, 8) * usbApdoMilivoltPerStep;
  pdpPower_mW = EXTRACT_BIT_RANGE(val, 7, 0) * usbApdoMiliwattPerStep;
}

AugmentedSourcePDO::AugmentedSourcePDO(uint32_t val) {
  type = (APDOType)EXTRACT_BIT_RANGE(val, 29, 28);

  switch (type) {
    case APDOType_SPRProgrammablePowerSupply:
      ppsPdo = SPRProgrammablePowerSupplySourcePDO(val);
      break;

    case APDOType_EPRAdjustableVoltageSupply:
      avsPdo = EPRAdjustableVoltageSupplySourcePDO(val);
      break;

    default:
      type = NUM_APDO_TYPE;
  }
}

SourcePDO::SourcePDO(uint32_t val) {
  raw = val;
  type = (PDOType)EXTRACT_BIT_RANGE(val, 31, 30);

  switch (type) {
    case PDOType_FixedSupply:
      fixedSupplyPdo = FixedSupplySourcePDO(val);
      break;

    case PDOType_Battery:
      batteryPdo = BatterySourcePDO(val);
      break;

    case PDOType_VariableSupply:
      variableSupplyPdo = VariableSupplySourcePDO(val);
      break;

    case PDOType_AugmentedPDO:
      augmentedPdo = AugmentedSourcePDO(val);
      break;

    default:
      // TODO: log error?
      type = NUM_PDO_TYPE;
  }
}

Header::Header(SOPType detectedSop, uint16_t val) {
  sop = detectedSop;
  numberOfDataObjects = EXTRACT_BIT_RANGE(val, 14, 12);  // Bits 14..12 == Number of Data Objects
  messageId = EXTRACT_BIT_RANGE(val, 11, 9);             // Bits 11..9 == Message ID
  portPowerRoleOrCablePlug =
      CHECK_BIT(val, 8);  // Bit 8 == Port Power Role (SOP only) / Cable Plug (SOP' and SOP" only)
  specRev = (PDSpecRevision)EXTRACT_BIT_RANGE(val, 7, 6);      // Bits 7..6 == Spec Revision
  portDataRole = CHECK_BIT(val, 5);            // Bit 5 == Port Data Role (SOP only)
  messageType = EXTRACT_BIT_RANGE(val, 4, 0);  // Bits 4..0 == Message Type
}

FixedSupplyRequest::FixedSupplyRequest(uint32_t val) {
  giveBack = CHECK_BIT(val, 27);
  capabilityMismatch = CHECK_BIT(val, 26);
  usbCommunicationCapable = CHECK_BIT(val, 25);
  noUsbSuspend = CHECK_BIT(val, 24);
  unchunkedExtendedMessagesSupported = CHECK_BIT(val, 23);
  eprModeCapable = CHECK_BIT(val, 22);

  operatingCurrent_mA = EXTRACT_BIT_RANGE(val, 19, 10) * usbPdoMiliampPerStep;

  if (giveBack) {
    maxOperatingCurrent_mA = EXTRACT_BIT_RANGE(val, 9, 0) * usbPdoMiliampPerStep;
  } else {
    minOperatingCurrent_mA = EXTRACT_BIT_RANGE(val, 9, 0) * usbPdoMiliampPerStep;
  }
}

BatterySupplyRequest::BatterySupplyRequest(uint32_t val) {
  giveBack = CHECK_BIT(val, 27);
  capabilityMismatch = CHECK_BIT(val, 26);
  usbCommunicationCapable = CHECK_BIT(val, 25);
  noUsbSuspend = CHECK_BIT(val, 24);
  unchunkedExtendedMessagesSupported = CHECK_BIT(val, 23);
  eprModeCapable = CHECK_BIT(val, 22);

  operatingPower_mW = EXTRACT_BIT_RANGE(val, 19, 10) * usbPdoMiliwattPerStep;

  if (giveBack) {
    maxOperatingPower_mW = EXTRACT_BIT_RANGE(val, 9, 0) * usbPdoMiliwattPerStep;
  } else {
    minOperatingPower_mW = EXTRACT_BIT_RANGE(val, 9, 0) * usbPdoMiliwattPerStep;
  }
}

SPRProgrammablePowerSupplyRequest::SPRProgrammablePowerSupplyRequest(uint32_t val) {
  capabilityMismatch = CHECK_BIT(val, 26);
  usbCommunicationCapable = CHECK_BIT(val, 25);
  noUsbSuspend = CHECK_BIT(val, 24);
  unchunkedExtendedMessagesSupported = CHECK_BIT(val, 23);
  eprModeCapable = CHECK_BIT(val, 22);

  // TODO: eliminate magic number. USB-PD spec says this is in 20mV units? But the corresponding
  // APDO uses 100mV units
  outputVoltage_mV = EXTRACT_BIT_RANGE(val, 20, 9) * 20;
  operatingCurrent_mA = EXTRACT_BIT_RANGE(val, 6, 0) * usbApdoMiliampPerStep;
}

EPRAdjustableVoltageSupplyRequest::EPRAdjustableVoltageSupplyRequest(uint32_t val) {
  capabilityMismatch = CHECK_BIT(val, 26);
  usbCommunicationCapable = CHECK_BIT(val, 25);
  noUsbSuspend = CHECK_BIT(val, 24);
  unchunkedExtendedMessagesSupported = CHECK_BIT(val, 23);
  eprModeCapable = CHECK_BIT(val, 22);

  // TODO: eliminate magic number. USB-PD spec says this is in 25mV units? But the corresponding
  // APDO uses 100mV units
  outputVoltage_mV = EXTRACT_BIT_RANGE(val, 20, 9) * 25;
  operatingCurrent_mA = EXTRACT_BIT_RANGE(val, 6, 0) * usbApdoMiliampPerStep;
}

Request::Request(SourcePDO& pdo, uint32_t val) {
  type = pdo.type;
  raw = val;

  switch(type) {
    case PDOType_FixedSupply:
      fixedSupplyRequest = FixedSupplyRequest(val);
      break;

    case PDOType_Battery:
      batterySupplyRequest = BatterySupplyRequest(val);
      break;

    case PDOType_VariableSupply:
      variableSupplyRequest = VariableSupplyRequest(val);
      break;

    case PDOType_AugmentedPDO:
      switch (pdo.augmentedPdo.type) {
        case APDOType_SPRProgrammablePowerSupply:
          ppsRequest = SPRProgrammablePowerSupplyRequest(val);
          break;

        case APDOType_EPRAdjustableVoltageSupply:
          avsRequest = EPRAdjustableVoltageSupplyRequest(val);
          break;

        default:
          // TODO: log error?
          type = NUM_PDO_TYPE;
      }
      break;

    default:
      // TODO: log error?
      type = NUM_PDO_TYPE;
  }
}

StructuredVDM::StructuredVDM(uint32_t val) {
  version = (StructuredVDMVersion)EXTRACT_BIT_RANGE(val, 14, 13);
  objectPosition = EXTRACT_BIT_RANGE(val, 10, 8);
  commandType = (StructuredVDMCommandType)EXTRACT_BIT_RANGE(val, 7, 6);
  command = (StructuredVDMCommand)EXTRACT_BIT_RANGE(val, 4, 0);
}

VDMHeader::VDMHeader(uint32_t val) {
  vid = EXTRACT_BIT_RANGE(val, 31, 16);
  type = (VDMType)CHECK_BIT(val, 15);

  if (type == VDMType_Structured) {
    structuredData = StructuredVDM(val);
  } else {
    unstructuredData = EXTRACT_BIT_RANGE(val, 14, 0);
  }
}

IDHeaderVdo::IDHeaderVdo(SOPType sop, uint32_t val) {
  sopType = sop;

  usbHostCommunicationCapable = CHECK_BIT(val, 31);
  usbDeviceCommunicationCapable = CHECK_BIT(val, 30);

  if (sopType == SOPType_SOP) {
    sopProductTypeUfp = (SOPProductTypeUfp)EXTRACT_BIT_RANGE(val, 29, 27);
  } else {
    sopPrimeProductType = (SOPPrimeProductType)EXTRACT_BIT_RANGE(val, 29, 27);
  }

  modalOperationSupported = CHECK_BIT(val, 26);

  sopProductTypeDfp = (SOPProductTypeDfp)EXTRACT_BIT_RANGE(val, 25, 23);
  connectorType = (ConnectorType)EXTRACT_BIT_RANGE(val, 22, 21);

  vid = EXTRACT_BIT_RANGE(val, 15, 0);
}

ProductVdo::ProductVdo(uint32_t val) {
  pid = EXTRACT_BIT_RANGE(val, 31, 16);
  bcdDevice = EXTRACT_BIT_RANGE(val, 15, 0);
}