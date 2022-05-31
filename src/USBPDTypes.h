#ifndef USBPD_TYPES_H
#define USBPD_TYPES_H

#define CHECK_BIT(val, bit) (((val) & (1 << (bit))) != 0)
#define EXTRACT_BIT_RANGE(val, msb, lsb) \
  ((((val) & ((0xFFFFFFFF >> (32 - (msb + 1))) & (0xFFFFFFFF << (lsb))))) >> (lsb))

enum FrameType {
  FRAME_TYPE_PREAMBLE,

  FRAME_TYPE_SOP,
  FRAME_TYPE_SOP_PRIME,
  FRAME_TYPE_SOP_DOUBLE_PRIME,
  FRAME_TYPE_SOP_PRIME_DEBUG,
  FRAME_TYPE_SOP_DOUBLE_PRIME_DEBUG,
  FRAME_TYPE_SOP_ERROR,

  FRAME_TYPE_HEADER,

  FRAME_TYPE_CRC32,

  FRAME_TYPE_EOP,

  FRAME_TYPE_BYTE,

  FRAME_TYPE_GENERIC_DATA_OBJECT,

  FRAME_TYPE_SOURCE_POWER_DATA_OBJECT,

  FRAME_TYPE_REQUEST_DATA_OBJECT,

  FRAME_TYPE_VDM_HEADER,

  NUM_FRAME_TYPE
};

enum KCODEType {
  KCODEType_SYNC_1,
  KCODEType_SYNC_2,
  KCODEType_RST_1,
  KCODEType_RST_2,
  KCODEType_EOP,
  KCODEType_SYNC_3,

  NUM_KCODE
};

static const int numKcodeBits = 5;
static const uint8_t kcode_map[NUM_KCODE] = {
    0x18, // Sync-1 = 11000
    0x11, // Sync-2 = 10001
    0x07, // RST-1  = 00111
    0x19, // RST-2  = 11001
    0x0D, // EOP    = 01101
    0x06, // Sync-3 = 00110
};

enum SOPType {
  SOPType_SOP,
  SOPType_SOP_PRIME,
  SOPType_SOP_DOUBLE_PRIME,
  SOPType_SOP_PRIME_DEBUG,
  SOPType_SOP_DOUBLE_PRIME_DEBUG,

  NUM_SOP_TYPE
};

const int numKcodeInSOP = 4;
static const KCODEType sop_map[NUM_SOP_TYPE][numKcodeInSOP] = {
    {KCODEType_SYNC_1, KCODEType_SYNC_1, KCODEType_SYNC_1, KCODEType_SYNC_2}, // SOP
    {KCODEType_SYNC_1, KCODEType_SYNC_1, KCODEType_SYNC_3, KCODEType_SYNC_3}, // SOP'
    {KCODEType_SYNC_1, KCODEType_SYNC_3, KCODEType_SYNC_1, KCODEType_SYNC_3}, // SOP"
    {KCODEType_SYNC_1, KCODEType_RST_2,  KCODEType_RST_2,  KCODEType_SYNC_3}, // SOP' Debug
    {KCODEType_SYNC_1, KCODEType_RST_2,  KCODEType_SYNC_3, KCODEType_SYNC_2}, // SOP" Debug
};

static const uint8_t fourBitToFiveBitLUT[16] = {
    // case 0x0:
    0x1E, // 11110

    // case 0x1:
    0x09, // 01001

    // case 0x2:
    0x14, // 10100

    // case 0x3:
    0x15, // 10101

    // case 0x4:
    0x0A, // 01010

    // case 0x5:
    0x0B, // 01011

    // case 0x6:
    0x0E, // 01110

    // case 0x7:
    0x0F, // 01111

    // case 0x8:
    0x12, // 10010

    // case 0x9:
    0x13, // 10011

    // case 0xA:
    0x16, // 10110

    // case 0xB:
    0x17, // 10111

    // case 0xC:
    0x1A, // 11010

    // case 0xD:
    0x1B, // 11011

    // case 0xE:
    0x1C, // 11100

    // case 0xF:
    0x1D, // 11101
};

enum ControlMessageTypes {
    ControlMessage_Reserved,
    ControlMessage_GoodCRC,
    ControlMessage_GotoMin,
    ControlMessage_Accept,
    ControlMessage_Reject,
    ControlMessage_Ping,
    ControlMessage_PS_RDY,
    ControlMessage_Get_Source_Cap,
    ControlMessage_Get_Sink_Cap,
    ControlMessage_DR_Swap,
    ControlMessage_PR_Swap,
    ControlMessage_VCONN_Swap,
    ControlMessage_Wait,
    ControlMessage_Soft_Reset,
    ControlMessage_Data_Reset,
    ControlMessage_Data_Reset_Complete,
    ControlMessage_Not_Supported,
    ControlMessage_Get_Source_Cap_Extended,
    ControlMessage_Get_Status,
    ControlMessage_FR_Swap,
    ControlMessage_Get_PPS_Status,
    ControlMessage_Get_Country_Codes,
    ControlMessage_Get_Sink_Cap_Extended,
    ControlMessage_Get_Source_Info,
    ControlMessage_Get_Revision,

    NUM_CONTROL_MESSAGE
};

static const char* ControlMessageNames[NUM_CONTROL_MESSAGE] = {
    "ControlMessage_Reserved",
    "ControlMessage_GoodCRC",
    "ControlMessage_GotoMin",
    "ControlMessage_Accept",
    "ControlMessage_Reject",
    "ControlMessage_Ping",
    "ControlMessage_PS_RDY",
    "ControlMessage_Get_Source_Cap",
    "ControlMessage_Get_Sink_Cap",
    "ControlMessage_DR_Swap",
    "ControlMessage_PR_Swap",
    "ControlMessage_VCONN_Swap",
    "ControlMessage_Wait",
    "ControlMessage_Soft_Reset",
    "ControlMessage_Data_Reset",
    "ControlMessage_Data_Reset_Complete",
    "ControlMessage_Not_Supported",
    "ControlMessage_Get_Source_Cap_Extended",
    "ControlMessage_Get_Status",
    "ControlMessage_FR_Swap",
    "ControlMessage_Get_PPS_Status",
    "ControlMessage_Get_Country_Codes",
    "ControlMessage_Get_Sink_Cap_Extended",
    "ControlMessage_Get_Source_Info",
    "ControlMessage_Get_Revision",
};

enum DataMessageTypes {
    DataMessage_Reserved,
    DataMessage_Source_Capabilities,
    DataMessage_Request,
    DataMessage_BIST,
    DataMessage_Sink_Capabilities,
    DataMessage_BatteryStatus,
    DataMessage_Alert,
    DataMessage_Get_Country_Info,
    DataMessage_Enter_USB,
    DataMessage_EPR_Request,
    DataMessage_EPR_Mode,
    DataMessage_Source_Info,
    DataMessage_Revision,
    DataMessage_Reserved13,
    DataMessage_Reserved14,
    DataMessage_Vendor_Defined,

    NUM_DATA_MESSAGE
};

static const char* DataMessageNames[NUM_DATA_MESSAGE] = {
    "DataMessage_Reserved",
    "DataMessage_Source_Capabilities",
    "DataMessage_Request",
    "DataMessage_BIST",
    "DataMessage_Sink_Capabilities",
    "DataMessage_BatteryStatus",
    "DataMessage_Alert",
    "DataMessage_Get_Country_Info",
    "DataMessage_Enter_USB",
    "DataMessage_EPR_Request",
    "DataMessage_EPR_Mode",
    "DataMessage_Source_Info",
    "DataMessage_Revision",
    "DataMessage_Reserved13",
    "DataMessage_Reserved14",
    "DataMessage_Vendor_Defined",
};

enum PDSpecRevision {
    PDSpecRevision_1P0,
    PDSpecRevision_2P0,
    PDSpecRevision_3P0,

    NUM_PD_SPEC_REVISION
};

enum PortDataRole {
    PortDataRole_UFP, // Upstream-facing Port, the USB Device
    PortDataRole_DFP, // Downstream-facing Port, the USB Host

    NUM_PORT_DATA_ROLE
};

enum PortPowerRole {
    PortPowerRole_Sink, // Device will draw power
    PortPowerRole_Source, // Device will provide power

    NUM_PORT_POWER_ROLE
};

enum CablePlug {
    CablePlug_MsgSrcPort, // Message came from the UFP or DFP port
    CablePlug_MsgSrcPlug, // Message came from one of the plugs

    NUM_CABLE_PLUG
};

static const uint32_t usbCrcPolynomial = 0x04C11DB7;

enum PDOType {
    PDOType_FixedSupply,
    PDOType_Battery,
    PDOType_VariableSupply,
    PDOType_AugmentedPDO,

    NUM_PDO_TYPE
};

enum APDOType {
    APDOType_SPRProgrammablePowerSupply,
    APDOType_EPRAdjustableVoltageSupply,

    NUM_APDO_TYPE
};

enum VDMType {
    VDMType_Unstructured,
    VDMType_Structured,

    NUM_VDM_TYPE
};

enum StructuredVDMVersion {
    StructuredVDMVersion_1P0,
    StructuredVDMVersion_2P0,

    NUM_STRUCTURED_VDM_VERSION
};

static const char*StructuredVDMVersionNames[NUM_STRUCTURED_VDM_VERSION] = {
    "1.0",
    "2.0",
};

enum StructuredVDMCommandType {
    StructuredVDMCommandType_REQ,
    StructuredVDMCommandType_ACK,
    StructuredVDMCommandType_NAK,
    StructuredVDMCommandType_BUSY,

    NUM_STRUCTURED_VDM_COMMAND_TYPE
};

static const char* StructuredVDMCommandTypeNames[NUM_STRUCTURED_VDM_COMMAND_TYPE] = {
    "REQ",
    "ACK",
    "NAK",
    "BUSY",
};

enum StructuredVDMCommand {
    StructuredVDMCommand_Reserved,
    StructuredVDMCommand_DiscoverIdentity,
    StructuredVDMCommand_DiscoverSVIDs,
    StructuredVDMCommand_DiscoverModes,
    StructuredVDMCommand_EnterMode,
    StructuredVDMCommand_ExitMode,
    StructuredVDMCommand_Attention,
    StructuredVDMCommand_Reserved7,
    StructuredVDMCommand_Reserved8,
    StructuredVDMCommand_Reserved9,
    StructuredVDMCommand_Reserved10,
    StructuredVDMCommand_Reserved11,
    StructuredVDMCommand_Reserved12,
    StructuredVDMCommand_Reserved13,
    StructuredVDMCommand_Reserved14,
    StructuredVDMCommand_Reserved15,
    StructuredVDMCommand_SVID16,
    StructuredVDMCommand_SVID17,
    StructuredVDMCommand_SVID18,
    StructuredVDMCommand_SVID19,
    StructuredVDMCommand_SVID20,
    StructuredVDMCommand_SVID21,
    StructuredVDMCommand_SVID22,
    StructuredVDMCommand_SVID23,
    StructuredVDMCommand_SVID24,
    StructuredVDMCommand_SVID25,
    StructuredVDMCommand_SVID26,
    StructuredVDMCommand_SVID27,
    StructuredVDMCommand_SVID28,
    StructuredVDMCommand_SVID29,
    StructuredVDMCommand_SVID30,
    StructuredVDMCommand_SVID31,

    NUM_STRUCTURED_VDM_COMMAND
};

static const char* StructuredVDMCommandNames[NUM_STRUCTURED_VDM_COMMAND] = {
    "Reserved",
    "DiscoverIdentity",
    "DiscoverSVIDs",
    "DiscoverModes",
    "EnterMode",
    "ExitMode",
    "Attention",
    "Reserved7",
    "Reserved8",
    "Reserved9",
    "Reserved10",
    "Reserved11",
    "Reserved12",
    "Reserved13",
    "Reserved14",
    "Reserved15",
    "SVID16",
    "SVID17",
    "SVID18",
    "SVID19",
    "SVID20",
    "SVID21",
    "SVID22",
    "SVID23",
    "SVID24",
    "SVID25",
    "SVID26",
    "SVID27",
    "SVID28",
    "SVID29",
    "SVID30",
    "SVID31",
};

enum SOPProductTypeUfp {
  SOPProductTypeUfp_NotUFP,
  SOPProductTypeUfp_PDUSBHub,
  SOPProductTypeUfp_PDUSBPeripheral,
  SOPProductTypeUfp_PSD,

  NUM_SOP_PRODUCT_TYPE_UFP
};

enum SOPProductTypeDfp {
  SOPProductTypeUfp_NotDFP,
  SOPProductTypeUfp_PDUSBHub,
  SOPProductTypeUfp_PDUSBHost,
  SOPProductTypeUfp_PowerBrick,

  NUM_SOP_PRODUCT_TYPE_UFP
};

enum SOPPrimeProductType {
  SOPPrimeProductType_NotCablePlug_VPD, // VPD == VCONN Powered Device
  SOPPrimeProductType_Reserved1,
  SOPPrimeProductType_Reserved2,
  SOPPrimeProductType_PassiveCable,
  SOPPrimeProductType_ActiveCable,
  SOPPrimeProductType_Reserved5,
  SOPPrimeProductType_VCONNPoweredDevice,

  NUM_SOP_PRIME_PRODUCT_TYPE
};

enum ConnectorType {
  ConnectorType_ReservedLegacy,
  ConnectorType_Reserved1,
  ConnectorType_USBCReceptable,
  ConnectorType_USBCPlug,

  NUM_CONNECTOR_TYPE
};

enum UFPVDOVersion {
  UFPVDOVersion_Reserved0,
  UFPVDOVersion_Reserved1,
  UFPVDOVersion_Reserved2,
  UFPVDOVersion_1P3,

  NUM_UFP_VDO_VERSION
};

enum VCONNPower {
  VCONNPower_1W,
  VCONNPower_1P5W,
  VCONNPower_2W,
  VCONNPower_3W,
  VCONNPower_4W,
  VCONNPower_5W,
  VCONNPower_6W,

  NUM_VCONN_POWER
};

enum USBHighestSpeed {
  USBHighestSpeed_2P0,
  USBHighestSpeed_3P2_Gen1,
  USBHighestSpeed_3P2_4P0_Gen2,
  USBHighestSpeed_4P0_Gen3,

  NUM_USB_HIGHEST_SPEED
};

// Constants for PDOs
static const int usbPdoMilivoltPerStep = 50;
static const int usbPdoMiliampPerStep = 10;
static const int usbPdoMiliwattPerStep = 250;

// Constants for APDOs
static const int usbApdoMilivoltPerStep = 100;
static const int usbApdoMiliampPerStep = 50;
static const int usbApdoMiliwattPerStep = 1000;

#endif // USBPD_TYPES_H
