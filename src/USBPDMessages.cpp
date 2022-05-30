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
  messageType = EXTRACT_BIT_RANGE(val, 3, 0);  // Bits 3..0 == Message Type
}
