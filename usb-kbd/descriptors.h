uint8_t deviceDescriptor[] = {
    18,            // length of this descriptor
    0x01,          // DEVICE Descriptor Type
    0x00, 0x01,    // USB version 1.0
    0,             // Device Class per interface
    0,    0,       // subclass, protocol 0,0
    64,            // Max Packet Size ep0
    0x8a, 0x2e,    // VendorID  = 0x2e8a (Raspberry Pi)
    0x1f, 0x10,    // ProductID = 0x101f (RP2040-Zero)
    0x10, 0x00,    // Device Version 0.10
    1,    2,    3, // Manufacturer/Product/Serial strings
    1,             // NumConfigurations
};

uint8_t configDescriptor[] = {
    // Config 0 header
    9,                                //  Length
    0x02,                             //  CONFIGURATION Descriptor Type
    34, 0,                            //  TotalLength
    1,                                //  NumInterfaces
    1,                                //  ConfigurationValue
    0,                                //  Configuration string not set
    0x80,                             //  Bus powered, no remote wakeup
    50,                               //  MaxPower 100mA

    // interface 0
    9,    // Length
    0x04, // INTERFACE Descriptor Type
    0, 0, // Interface Number, Alternate Setting
    1,    // Num Endpoints
    0x03, // InterfaceClass: Human Interface Device
    0,    // InterfaceSubClass
    1,    // InterfaceProtocol Keyboard
    0,    // Interface string not set

    //HID Device Descriptor
    9,          //Length
    0x21,       //HID Descriptor Type
    0x10, 0x01, //bcdHID 1.10
    0,          //country code
    1,          //Num Descriptors
    0x22,       //Descriptor Type REPORT
    25,         //Report Descriptor length
    0,          //

    // endpoint 0x81
    7,     //  Length
    0x05,  //  ENDPOINT Descriptor Type
    0x81,  //  Endpoint Address 1-IN
    0x03,  //  Attributes: INTERRUPT
    8, 0,  //  MaxPacketSize
    0x0a,  //  Interval
};

uint8_t reportDescriptor[] = { //comment obtained with hid-decode
    0x05, 0x01,                    // Usage Page (Generic Desktop)        0
    0x09, 0x06,                    // Usage (Keyboard)                    2
    0xa1, 0x01,                    // Collection (Application)            4
    0x05, 0x07,                    //  Usage Page (Keyboard)              6
    0x19, 0x00,                    //  Usage Minimum (0)                  8
    0x2a, 0xff, 0x00,              //  Usage Maximum (255)                10
    0x15, 0x00,                    //  Logical Minimum (0)                13
    0x26, 0xff, 0x00,              //  Logical Maximum (255)              15
    0x95, 0x01,                    //  Report Count (1)                   18
    0x75, 0x08,                    //  Report Size (8)                    20
    0x81, 0x00,                    //  Input (Data,Arr,Abs)               22
    0xc0,                          // End Collection                      24
};

uint8_t langidDescriptor[] = {

    6, 0x03, 0x09, 0x04, 0x13, 0x04, //English (United States) and Dutch (Netherlands)
};

uint8_t manufacturerDescriptor[] = {

    10, 0x03, 'A', 0, 'C', 0, 'M', 0, 'E', 0
};

uint8_t productDescriptor[] = {

    18, 0x03, 'K', 0, 'e', 0, 'y', 0, 'b', 0, 'o', 0, 'a', 0, 'r', 0, 'd', 0
};

uint8_t serialDescriptor[] = {

    12, 0x03, 'V', 0, '2', 0, '.', 0, '1', 0, '7', 0
};
