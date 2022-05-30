#ifndef USBPD_MESSAGES_H
#define USBPD_MESSAGES_H

#include <cstdint>

#include "USBPDTypes.h"

namespace USBPDMessages {

struct FixedSupplySourcePDO {
  FixedSupplySourcePDO(uint32_t value);

  bool dualRolePower;
  bool usbSuspendSupported;
  bool unconstrainedPower;
  bool usbCommunicationsCapable;
  bool dualRoleData;
  bool unchunkedExtendedMessagesSupported;
  bool eprModeCapable;
  uint32_t peakCurrentMode;
  uint32_t voltage_mV;
  uint32_t maxCurrent_mA;
};

struct BatterySourcePDO {
  BatterySourcePDO(uint32_t value);

  uint32_t maxVoltage_mV;
  uint32_t minVoltage_mV;
  uint32_t maxPower_mW;
};

struct VariableSupplySourcePDO {
  VariableSupplySourcePDO(uint32_t value);

  uint32_t maxVoltage_mV;
  uint32_t minVoltage_mV;
  uint32_t maxCurrent_mA;
};

struct SPRProgrammablePowerSupplySourcePDO {
  SPRProgrammablePowerSupplySourcePDO(uint32_t value);

  bool ppsPowerLimited;
  uint32_t maxVoltage_mV;
  uint32_t minVoltage_mV;
  uint32_t maxCurrent_mA;
};

struct EPRAdjustableVoltageSupplySourcePDO {
  EPRAdjustableVoltageSupplySourcePDO(uint32_t value);

  uint32_t peakCurrentMode;
  uint32_t maxVoltage_mV;
  uint32_t minVoltage_mV;
  uint32_t pdpPower_mW;
};

struct AugmentedSourcePDO {
  AugmentedSourcePDO(uint32_t value);

  APDOType type;

  union {
    SPRProgrammablePowerSupplySourcePDO ppsPdo;
    EPRAdjustableVoltageSupplySourcePDO avsPdo;
  };
};

struct SourcePDO {
  SourcePDO() = delete;
  SourcePDO(uint32_t value);

  PDOType type;
  uint32_t raw;

  union {
    FixedSupplySourcePDO fixedSupplyPdo;
    BatterySourcePDO batteryPdo;
    VariableSupplySourcePDO variableSupplyPdo;
    AugmentedSourcePDO augmentedPdo;
  };
};

struct Header {
  Header() = delete;
  Header(SOPType sop, uint16_t val);

  SOPType sop;
  uint8_t numberOfDataObjects;
  uint8_t messageId;
  bool portPowerRoleOrCablePlug;
  PDSpecRevision specRev;
  bool portDataRole;
  uint8_t messageType;
};

struct FixedSupplyRequest {
  FixedSupplyRequest(uint32_t val);

  bool giveBack;
  bool capabilityMismatch;
  bool usbCommunicationCapable;
  bool noUsbSuspend;
  bool unchunkedExtendedMessagesSupported;
  bool eprModeCapable;

  uint32_t operatingCurrent_mA;

  union {
    uint32_t maxOperatingCurrent_mA;  // Max if giveBack is false
    uint32_t minOperatingCurrent_mA;  // Min if giveBack is true
  };
};

// Identical per PD Spec
typedef FixedSupplyRequest VariableSupplyRequest;

struct BatterySupplyRequest {
  BatterySupplyRequest(uint32_t val);

  bool giveBack;
  bool capabilityMismatch;
  bool usbCommunicationCapable;
  bool noUsbSuspend;
  bool unchunkedExtendedMessagesSupported;
  bool eprModeCapable;

  uint32_t operatingPower_mW;

  union {
    uint32_t maxOperatingPower_mW;  // Max if giveBack is false
    uint32_t minOperatingPower_mW;  // Min if giveBack is true
  };
};

struct SPRProgrammablePowerSupplyRequest {
  SPRProgrammablePowerSupplyRequest(uint32_t val);

  bool capabilityMismatch;
  bool usbCommunicationCapable;
  bool noUsbSuspend;
  bool unchunkedExtendedMessagesSupported;
  bool eprModeCapable;

  uint32_t outputVoltage_mV;
  uint32_t operatingCurrent_mA;
};

struct EPRAdjustableVoltageSupplyRequest {
  EPRAdjustableVoltageSupplyRequest(uint32_t val);

  bool capabilityMismatch;
  bool usbCommunicationCapable;
  bool noUsbSuspend;
  bool unchunkedExtendedMessagesSupported;
  bool eprModeCapable;

  uint32_t outputVoltage_mV;
  uint32_t operatingCurrent_mA;
};

struct Request {
  Request() = delete;
  Request(SourcePDO& pdo, uint32_t val);

  uint32_t raw;
  PDOType type;

  union {
    FixedSupplyRequest fixedSupplyRequest;
    VariableSupplyRequest variableSupplyRequest;
    BatterySupplyRequest batterySupplyRequest;
    SPRProgrammablePowerSupplyRequest ppsRequest;
    EPRAdjustableVoltageSupplyRequest avsRequest;
  };
};

struct StructuredVDM {
  StructuredVDM(uint32_t val);

  StructuredVDMVersion version;
  uint8_t objectPosition;
  StructuredVDMCommandType commandType;
  StructuredVDMCommand command;
};

struct VDMHeader {
  VDMHeader() = delete;
  VDMHeader(uint32_t val);

  VDMType type;
  uint16_t vid;

  union {
    uint16_t unstructuredData;
    StructuredVDM structuredData;
  };
};

};  // namespace USBPDMessages

#endif  // USBPD_MESSAGES_H
