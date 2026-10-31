uint8_t deviceDescriptor[] = {
    18,            // length of this descriptor
    0x01,          // DEVICE Descriptor Type
    0x00, 0x01,    // USB version 1.0
    0,             // Device Class per interface
    0,    0,       // subclass, protocol
    64,            // Max Packet Size ep0
    0x8a, 0x2e,    // VendorID  = 0x2e8a (Raspberry Pi)
    0x1f, 0x10,    // ProductID = 0x101f (RP2040-Zero)
    0x00, 0x00,    // Device Version 0.0
    0,    0,    0, // Device strings not set
    1,             // NumConfigurations
};

uint8_t configDescriptor[] = {
    // Config 0 header
    9,                                //  Length
    0x02,                             //  CONFIGURATION Descriptor Type
    18, 0,                            //  TotalLength
    1,                                //  NumInterfaces
    1,                                //  ConfigurationValue
    0,                                //  Configuration string not set
    0x80,                             //  Bus powered, no remote wakeup
    50,                               //  MaxPower 100mA

    // interface 0
    9,    // Length
    0x04, // INTERFACE Descriptor Type
    0, 0, // Interface Number, Alternate Setting
    0,    // Num Endpoints
    0x00, // InterfaceClass
    0,    // InterfaceSubClass
    0x00, // InterfaceProtocol
    0,    // Interface string not set
};
