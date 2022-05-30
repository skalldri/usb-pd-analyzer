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

};

#endif  // USBPD_MESSAGES_H