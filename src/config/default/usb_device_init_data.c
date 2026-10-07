/*******************************************************************************
  USB Device Init Data — Vendor Bulk class (matches PersonDetection pattern)

  VID 0x04D8  PID 0x0053
  One interface, two bulk endpoints: EP1 OUT + EP1 IN.
  No function driver (driver = NULL) — application manages endpoints directly
  via USB_DEVICE_EndpointWrite / USB_DEVICE_EndpointRead.
 *******************************************************************************/

#include "configuration.h"
#include "definitions.h"

/**************************************************
 * USB Device Layer Function Driver Registration Table
 * driver = NULL  ->  Vendor / no-class function
 **************************************************/
static const USB_DEVICE_FUNCTION_REGISTRATION_TABLE funcRegistrationTable[1] =
{
    {
        .configurationValue = 1,
        .interfaceNumber    = 0,
        .speed              = (USB_SPEED)((uint32_t)USB_SPEED_HIGH | (uint32_t)USB_SPEED_FULL),
        .numberOfInterfaces = 1,
        .funcDriverIndex    = 0,
        .driver             = NULL,
        .funcDriverInit     = NULL,
    },
};

/*******************************************
 *  USB Device Descriptor
 *******************************************/
static const USB_DEVICE_DESCRIPTOR usbDeviceDescriptor =
{
    0x12,                               // bLength
    (uint8_t)USB_DESCRIPTOR_DEVICE,     // bDescriptorType
    0x0200,                             // bcdUSB
    0x00,                               // bDeviceClass    (defined at interface level)
    0x00,                               // bDeviceSubClass
    0x00,                               // bDeviceProtocol
    USB_DEVICE_EP0_BUFFER_SIZE,         // bMaxPacketSize0
    0x04D8,                             // idVendor  (Microchip)
    0x0053,                             // idProduct (matches PersonDetection)
    0x0100,                             // bcdDevice
    0x01,                               // iManufacturer
    0x02,                               // iProduct
    0x00,                               // iSerialNumber
    0x01,                               // bNumConfigurations
};

/*******************************************
 *  USB Full Speed Configuration Descriptor
 *  Total = 9 (config) + 9 (intf) + 7 (EP OUT) + 7 (EP IN) = 32 bytes
 *******************************************/
static const uint8_t fullSpeedConfigurationDescriptor[] =
{
    /* Configuration Descriptor */
    0x09,
    (uint8_t)USB_DESCRIPTOR_CONFIGURATION,
    USB_DEVICE_16bitTo8bitArrange(32),  // wTotalLength = 32
    1,                                  // bNumInterfaces
    0x01,                               // bConfigurationValue
    0x00,                               // iConfiguration
    USB_ATTRIBUTE_DEFAULT | USB_ATTRIBUTE_SELF_POWERED,
    50,                                 // bMaxPower (100 mA)

    /* Interface Descriptor */
    0x09,
    USB_DESCRIPTOR_INTERFACE,
    0,                                  // bInterfaceNumber
    0,                                  // bAlternateSetting
    2,                                  // bNumEndpoints
    0xFF,                               // bInterfaceClass    (Vendor)
    0xFF,                               // bInterfaceSubClass (Vendor)
    0xFF,                               // bInterfaceProtocol (Vendor)
    0,                                  // iInterface

    /* Endpoint 1 OUT Descriptor */
    0x07,
    USB_DESCRIPTOR_ENDPOINT,
    1 | USB_EP_DIRECTION_OUT,           // EP1 OUT
    (uint8_t)USB_TRANSFER_TYPE_BULK,
    0x40, 0x00,                         // wMaxPacketSize = 64 (FS)
    1,                                  // bInterval

    /* Endpoint 1 IN Descriptor */
    0x07,
    USB_DESCRIPTOR_ENDPOINT,
    1 | USB_EP_DIRECTION_IN,            // EP1 IN
    (uint8_t)USB_TRANSFER_TYPE_BULK,
    0x40, 0x00,                         // wMaxPacketSize = 64 (FS)
    1,                                  // bInterval
};

static USB_DEVICE_CONFIGURATION_DESCRIPTORS_TABLE fullSpeedConfigDescSet[1] =
{
    fullSpeedConfigurationDescriptor
};

/**************************************
 *  String Descriptors
 *************************************/
static const struct { uint8_t bLength; uint8_t bDscType; uint16_t string[1]; }
sd000 = { (uint8_t)sizeof(sd000), (uint8_t)USB_DESCRIPTOR_STRING, {0x0409} };

static const struct { uint8_t bLength; uint8_t bDscType; uint16_t string[25]; }
sd001 = { (uint8_t)sizeof(sd001), (uint8_t)USB_DESCRIPTOR_STRING,
    {'M','i','c','r','o','c','h','i','p',' ','T','e','c','h','n','o','l','o','g','y',' ','I','n','c','.'} };

static const struct { uint8_t bLength; uint8_t bDscType; uint16_t string[22]; }
sd002 = { (uint8_t)sizeof(sd002), (uint8_t)USB_DESCRIPTOR_STRING,
    {'S','A','M',' ','E','5','4',' ','C','a','m','e','r','a',' ','S','t','r','e','a','m','e','r'} };

static USB_DEVICE_STRING_DESCRIPTORS_TABLE stringDescriptors[3] =
{
    (const uint8_t *const)&sd000,
    (const uint8_t *const)&sd001,
    (const uint8_t *const)&sd002,
};

/*******************************************
 * USB Device Layer Master Descriptor Table
 *******************************************/
static const USB_DEVICE_MASTER_DESCRIPTOR usbMasterDescriptor =
{
    &usbDeviceDescriptor,
    1,
    fullSpeedConfigDescSet,
    NULL,
    0,
    NULL,
    3,
    stringDescriptors,
    NULL,
    NULL,
};

/****************************************************
 * USB Device Layer Initialization Data
 ****************************************************/
const USB_DEVICE_INIT usbDevInitData =
{
    .registeredFuncCount  = 1,
    .registeredFunctions  = (USB_DEVICE_FUNCTION_REGISTRATION_TABLE *)funcRegistrationTable,
    .usbMasterDescriptor  = (USB_DEVICE_MASTER_DESCRIPTOR *)&usbMasterDescriptor,
    .deviceSpeed          = USB_SPEED_FULL,
    .driverIndex          = DRV_USBFSV1_INDEX_0,
    .usbDriverInterface   = DRV_USBFSV1_DEVICE_INTERFACE,
    .queueSizeEndpointRead  = 1,
    .queueSizeEndpointWrite = 1,
};
// </editor-fold>
