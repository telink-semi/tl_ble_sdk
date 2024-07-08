/********************************************************************************************************
 * @file    myudb_usbdesc.c
 *
 * @brief   This is the source file for BLE SDK
 *
 * @author  BLE GROUP
 * @date    06,2022
 *
 * @par     Copyright (c) 2022, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
 *
 *          Licensed under the Apache License, Version 2.0 (the "License");
 *          you may not use this file except in compliance with the License.
 *          You may obtain a copy of the License at
 *
 *              http://www.apache.org/licenses/LICENSE-2.0
 *
 *          Unless required by applicable law or agreed to in writing, software
 *          distributed under the License is distributed on an "AS IS" BASIS,
 *          WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *          See the License for the specific language governing permissions and
 *          limitations under the License.
 *
 *******************************************************************************************************/
#if(1)

#include "drivers.h"
#include "myudb.h"
#include "myudb_usbdesc.h"

// request parameters
/** Language descriptor structure. This descriptor, located in FLASH memory, is returned when the host requests
 *  the string descriptor with index 0 (the first index). It is actually an array of 16-bit integers, which indicate
 *  via the language ID table available at USB.org what languages the device supports for its string descriptors.
 */
static const USB_Descriptor_String_t language_desc = { {
        sizeof(USB_Descriptor_Header_t) + 2, DTYPE_String },
        { LANGUAGE_ID_ENG } };

/** Manufacturer descriptor string. This is a Unicode string containing the manufacturer's details in human readable
 *  form, and is read out upon request by the host when the appropriate string ID is requested, listed in the Device
 *  Descriptor.
 */
static const USB_Descriptor_String_t vendor_desc = { { sizeof(USB_Descriptor_Header_t)
        + sizeof(MYUDB_STRING_VENDOR) - 2, DTYPE_String }, // Header
        MYUDB_STRING_VENDOR };

/** Product descriptor string. This is a Unicode string containing the product's details in human readable form,
 *  and is read out upon request by the host when the appropriate string ID is requested, listed in the Device
 *  Descriptor.
 */
static const USB_Descriptor_String_t product_desc = { {
        sizeof(USB_Descriptor_Header_t) + sizeof(MYUDB_STRING_PRODUCT) - 2,
        DTYPE_String }, // Header
        MYUDB_STRING_PRODUCT };

/** Serial number string. This is a Unicode string containing the device's unique serial number, expressed as a
 *  series of uppercase hexadecimal digits.
 */
static const USB_Descriptor_String_t serial_desc = { { sizeof(USB_Descriptor_Header_t)
        + sizeof(MYUDB_STRING_SERIAL) - 2, DTYPE_String }, // Header
        MYUDB_STRING_SERIAL };


static const USB_Descriptor_Device_t device_desc = { {
        sizeof(USB_Descriptor_Device_t), DTYPE_Device }, // Header
        0x0200, // USBSpecification, USB 2.0
        USB_CSCP_NoDeviceClass, // Class
        USB_CSCP_NoDeviceSubclass, // SubClass
        USB_CSCP_NoDeviceProtocol, // Protocol
        8, // Endpoint0Size, Maximum Packet Size for Zero Endpoint. Valid Sizes are 8, 16, 32, 64
        MYUDB_ID_VENDOR, // VendorID
        MYUDB_ID_PRODUCT, // ProductID
        MYUDB_ID_VERSION/*0x0100*/, // .ReleaseNumber
        MYUDB_USB_STRING_VENDOR,     // .ManufacturerStrIndex
        MYUDB_USB_STRING_PRODUCT, // .ProductStrIndex
        0,     // .SerialNumStrIndex, iSerialNumber
        1 };

static const MYUDB_USB_Descriptor_Configuration_t
        configuration_desc = {
                {
                    {sizeof(USB_Descriptor_Configuration_Header_t),
                    DTYPE_Configuration }, // Length, type
                    sizeof(MYUDB_USB_Descriptor_Configuration_t), // TotalLength: variable
                    2, // NumInterfaces
                    1, // Configuration index
                    NO_DESCRIPTOR, // Configuration String
                    USB_CONFIG_ATTR_RESERVED, // Attributes
                    USB_CONFIG_POWER_MA(100) // MaxPower = 100mA
                },
                // printer_interface
                { { sizeof(USB_Descriptor_Interface_t), DTYPE_Interface },
                        0, 0, // AlternateSetting
                        2, // bNumEndpoints
                        PRNT_CSCP_PrinterClass, // bInterfaceclass ->Printer
                        PRNT_CSCP_PrinterSubclass, // bInterfaceSubClass -> Control
                        PRNT_CSCP_BidirectionalProtocol,// bInterfaceProtocol
                        NO_DESCRIPTOR // iInterface,  same as iProduct in USB_Descriptor_Device_t, or else not working
                },
                // printer_in_endpoint
                { { sizeof(USB_Descriptor_Endpoint_t), DTYPE_Endpoint }, // length, bDescriptorType
                        ENDPOINT_DIR_IN | MYUDB_EDP_IN_HCI, // endpoint id
                        EP_TYPE_BULK, // endpoint type
                        0x0040, // wMaxPacketSize
                        0 // bInterval
                },
                // printer_out_endpoint
                { { sizeof(USB_Descriptor_Endpoint_t), DTYPE_Endpoint }, // length, bDescriptorType
                        MYUDB_EDP_OUT_HCI, // endpoint id
                        EP_TYPE_BULK, // endpoint type
                        0x0040, // wMaxPacketSize
                        0 // polling bInterval. valid for iso or interrupt type
                },

                // printer_interface
                { { sizeof(USB_Descriptor_Interface_t), DTYPE_Interface },
                        1, 0, // AlternateSetting
                        2, // bNumEndpoints
                        PRNT_CSCP_PrinterClass, // bInterfaceclass ->Printer
                        PRNT_CSCP_PrinterSubclass, // bInterfaceSubClass -> Control
                        PRNT_CSCP_BidirectionalProtocol,// bInterfaceProtocol
                        NO_DESCRIPTOR // iInterface,  same as iProduct in USB_Descriptor_Device_t, or else not working
                },
                // printer_in_endpoint
                { { sizeof(USB_Descriptor_Endpoint_t), DTYPE_Endpoint }, // length, bDescriptorType
                        ENDPOINT_DIR_IN | MYUDB_EDP_IN_VCD, // endpoint id
                        EP_TYPE_BULK, // endpoint type
                        0x0040, // wMaxPacketSize
                        0 // bInterval
                },
                // printer_out_endpoint
                { { sizeof(USB_Descriptor_Endpoint_t), DTYPE_Endpoint }, // length, bDescriptorType
                        MYUDB_EDP_OUT_VCD, // endpoint id
                        EP_TYPE_BULK, // endpoint type
                        0x0040, // wMaxPacketSize
                        0 // polling bInterval. valid for iso or interrupt type
                },
        };

u8* myudb_usbdesc_get_language(void) {
    return (u8*)(size_t)(&language_desc);
}

u8* myudb_usbdesc_get_vendor(void) {
    return (u8*)(size_t)(&vendor_desc);
}

u8* myudb_usbdesc_get_product(void) {
    return (u8*)(size_t)(&product_desc);
}
u8* myudb_usbdesc_get_serial(void) {
    return (u8*)(size_t)(&serial_desc);
}

u8* myudb_usbdesc_get_device(void) {
    return (u8*)(u32) (&device_desc);
}

u8* myudb_usbdesc_get_configuration(void) {
    return (u8*)(u32) (&configuration_desc);
}

#endif


