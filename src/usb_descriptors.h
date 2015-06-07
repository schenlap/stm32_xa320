const struct usb_device_descriptor dev = {
        .bLength = USB_DT_DEVICE_SIZE,
        .bDescriptorType = USB_DT_DEVICE,
        .bcdUSB = 0x0200,      /* usb protocol verion 2 */
        .bDeviceClass = 0,     /* Class defined at interface level */
        .bDeviceSubClass = 0,
        .bDeviceProtocol = 0,
        .bMaxPacketSize0 = 64, /* max data soze for endpoint 0 */
        .idVendor = VENDOR_ID,
        .idProduct = PRODUCT_ID,
        .bcdDevice = 0x0100,   /* assigned by developer */
        .iManufacturer = 1,
        .iProduct = 2,
        .iSerialNumber = 3,
        .bNumConfigurations = 1,
};

/*
static const uint8_t hid_report_descriptor[] = {
	0x06, 0x00, 0xff, // USAGE_PAGE (Vendor Defined Page 1)
	0x09, 0x01, // USAGE (Vendor Usage 1)               
	0xa1, 0x01, // COLLECTION (Application)             
	            // ---- Common Definitions                   
	0x15, 0x00, //   LOGICAL MINIMUM(0)                 
	0x25, 0x01, //   LOGICAL MAXIMUM(1)                 
	0x75, 0x01, //   REPORT_SIZE(1)Button)              
	            // ---- Output report, body ----
	0x95, 0x06, //   REPORT_COUNT (6)
	0x05, 0x08, //   USAGE_PAGE (LEDs)
	0x09, 0x4b, //   USAGE (Generic Indicator)
	0x91, 0x02, //   OUTPUT (Data,Var,Abs)
	            // ---- Output report, padding ----
	0x95, 0x02, //   REPORT_COUNT (2)
	0x91, 0x03, //   OUTPUT (Cnst,Var,Abs)
	            // ---- Input report, body ----
	0x95, 0x04, //   REPORT_COUNT (4)
	0x05, 0x09, //   USAGE_PAGE (Button)
	0x19, 0x01, //   USAGE_MINIMUM (Button 1)
	0x29, 0x04, //   USAGE_MAXIMUM (Button 4)
	0x81, 0x02, //   INPUT (Data,Var,Abs)
	            // ---- Input report, padding ----
	0x81, 0x03, //   INPUT (Cnst,Var,Abs)
	0xc0        // END_COLLECTION
};
*/

static const uint8_t hid_report_descriptor[] = {
    // Usage Page = 0xFF00 (Vendor Defined Page 1)
    0x06, 0x00, 0xFF,
    // Usage (Vendor Usage 1)
    0x09, 0x01,
    // Collection (Application)
    0xA1, 0x01,
    //   Usage Minimum
    0x19, 0x01,
    //   Usage Maximum. 64 input usages total (0x01 to 0x40).
    0x29, 0x40,
    //   Logical Minimum (data bytes in the report may have minimum value = 0x00).
    0x15, 0x00,
    //   Logical Maximum (data bytes in the report may have
    //     maximum value = 0x00FF = unsigned 255).
    // TODO: Can this be one byte?
    0x26, 0xFF, 0x00,
    //   Report Size: 8-bit field size
    0x75, 0x08,
    //   Report Count: Make sixty-four 8-bit fields (the next time the parser hits
    //     an "Input", "Output", or "Feature" item).
    0x95, 0x40,
    //   Input (Data, Array, Abs): Instantiates input packet fields based on the
    //     above report size, count, logical min/max, and usage.
    0x81, 0x00,
    //   Usage Minimum
    0x19, 0x01,
    //   Usage Maximum. 64 output usages total (0x01 to 0x40)
    0x29, 0x40,
    //   Output (Data, Array, Abs): Instantiates output packet fields. Uses same
    //     report size and count as "Input" fields, since nothing new/different
    //     was specified to the parser since the "Input" item.
    0x91, 0x00,
    // End Collection
    0xC0,
};

static const uint8_t USBD_HID_Desc[] = {
  /* 18 */
  0x09,         /*bLength: HID Descriptor size*/
  0x21,         /*bDescriptorType: HID*/
  0x11,         /*bcdHID: HID Class Spec release number*/
  0x01,
  0x00,         /*bCountryCode: Hardware target country*/
  0x01,         /*bNumDescriptors: Number of HID class descriptors to follow*/
  0x22,         /*bDescriptorType*/
  65,           /*wItemLength: Total length of Report descriptor*/
  0x00,
};

static const struct {
	struct usb_hid_descriptor hid_descriptor;
	struct {
		uint8_t bReportDescriptorType;
		uint16_t wDescriptorLength;
	} __attribute__((packed)) hid_report;
} __attribute__((packed)) hid_function = {
	.hid_descriptor = {
		.bLength = sizeof(hid_function),
		.bDescriptorType = USB_DT_HID,
		.bcdHID = 0x0100,
		.bCountryCode = 0,
		.bNumDescriptors = 1,
	},
	.hid_report = {
		.bReportDescriptorType = USB_DT_REPORT,
		.wDescriptorLength = sizeof(hid_report_descriptor),
	},
};


const struct usb_endpoint_descriptor hid_endpoint[] = {{
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = 0x81, /* Bit 0..3b EP Number, Bit 4..6b Reserved = 0, Bit7 dir: 0 .. Out, 1 .. in -> IN Endpoint*/
	.bmAttributes = USB_ENDPOINT_ATTR_INTERRUPT,
	.wMaxPacketSize = 20,
	.bInterval = 10, /* Intervall for polling endpoint data transfers (1ms for low/full speed dev, 125us for high speed devices */
}, {
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = 0x01, /* Bit 0..3b EP Number, Bit 4..6b Reserved = 0, Bit7 dir: 0 .. Out, 1 .. in -> OUT Endpoint*/
	.bmAttributes = USB_ENDPOINT_ATTR_INTERRUPT,
	.wMaxPacketSize = 20,
	.bInterval = 10, /* Intervall for polling endpoint data transfers (1ms for low/full speed dev, 125us for high speed devices */
}};

const struct usb_interface_descriptor hid_iface = {
	.bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
	.bInterfaceNumber = 0,
	.bAlternateSetting = 0,
	.bNumEndpoints = 2, /* in EP, out EP */
	.bInterfaceClass = USB_CLASS_HID,
	.bInterfaceSubClass = 0, /* 0 .. none, 1 .. boot */
	.bInterfaceProtocol = 0, /* 0 .. none, 1 .. keyboard, 2 .. mouse */
	.iInterface = 0,

	.endpoint = hid_endpoint,

	.extra = &hid_function,
	.extralen = sizeof(hid_function),
};

const struct usb_interface ifaces[] = {{
	.num_altsetting = 1,
	.altsetting = &hid_iface,
}};

const struct usb_config_descriptor config = {
	.bLength = USB_DT_CONFIGURATION_SIZE,
	.bDescriptorType = USB_DT_CONFIGURATION,
	.wTotalLength = 0,
	.bNumInterfaces = 1,
	.bConfigurationValue = 1,
	.iConfiguration = 0,
	.bmAttributes = 0b11000000, /* D7 = fix 1 (USB 1.0 Bus Powered, D6 = self powered, D5 = remote waeup */
	.bMaxPower = 0x32,          /* max current in 2mA units 0x32 -> 100mA*/

	.interface = ifaces,
};

static const char *usb_strings[] = {
	"Stefan SCHOEFEGGER",	// iManufaturer
	"AX320",                // iProduct
	"0001",                 // iSerial
};
