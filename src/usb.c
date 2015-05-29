#include "gpio.h"
#include "usb.h"
#include "usb_descriptors.h"
//#include "systick.h"

/* Buffer to be used for control requests. */
uint8_t usbd_control_buffer[128];

uint8_t usb_ready = 0;

volatile uint32_t last_usb_request_time;

static uint8_t hid_buffer[4];

static void endpoint_callback(usbd_device *usbd_dev, uint8_t ep) {
    uint16_t bytes_read = usbd_ep_read_packet(usbd_dev,
                          ep,
                          hid_buffer,
                          sizeof(hid_buffer));
    (void)bytes_read;
	// TODO: handle incomind data
	gpio_toggle_led(LED5);
    // This function reads the packet and replaces it with the response buffer.
    //bool reboot = packet_handler(hid_buffer);
    // The full 64 bytes must be sent regardless of the amount of actual data.
	hid_buffer[0] = 'A';
	hid_buffer[1] = 'N';
	hid_buffer[2] = 'T';
	hid_buffer[3] = '*';
    usbd_ep_write_packet(usbd_dev, 0x81, hid_buffer, sizeof(hid_buffer));
//    if (reboot) {
//        for (volatile int i = 0; i < 800000; ++i);
//        scb_reset_system();
//    }
}

static int hid_control_request(usbd_device *usbd_dev, struct usb_setup_data *req, uint8_t **buf, uint16_t *len,
			void (**complete)(usbd_device *usbd_dev, struct usb_setup_data *req))
{
	(void)complete;
	(void)usbd_dev;

	/*
	if ((req->bmRequestType != 0x81) ||
		(req->bRequest != USB_REQ_GET_DESCRIPTOR) ||
		(req->wValue != 0x2200))
			return 0;
*/
	switch(req->bmRequestType) {
		case 0x81:
			switch(req->bRequest){
				case USB_REQ_GET_DESCRIPTOR:
					if(req->wValue==0x2200){
						*buf = (uint8_t *)hid_report_descriptor;
						*len = sizeof(hid_report_descriptor);
                        if(usb_ready==0)
								usb_ready=1;
						return 1;
					}else if(req->wValue==0x2100){
						*buf = (uint8_t *)USBD_HID_Desc;
						*len = sizeof(USBD_HID_Desc);
                        return 1;
					}
					return 0;
				default:
					return 0;
			}
			break;
		default:
			return 0;
	}

}

static void hid_set_config(usbd_device *usbd_dev, uint16_t wValue)
{
	(void)wValue;
	(void)usbd_dev;

	usbd_ep_setup(usbd_dev, 0x81, USB_ENDPOINT_ATTR_INTERRUPT, 64, NULL);
	usbd_ep_setup(usbd_dev, 0x01, USB_ENDPOINT_ATTR_INTERRUPT, 64, endpoint_callback);

	usbd_register_control_callback(
				usbd_dev,
				USB_REQ_TYPE_IN | USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_INTERFACE,
				USB_REQ_TYPE_DIRECTION | USB_REQ_TYPE_TYPE | USB_REQ_TYPE_RECIPIENT,
				hid_control_request);
}

usbd_device *my_usb_device;

void usb_send_packet(const void *buf, int len){
    //gpio_toggle_led(LED5);
    while(usbd_ep_write_packet(my_usb_device, 0x81, buf, len) == 0);
    //gpio_clear(LED5, 0);
}

void usb_setup(void)
{

	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_OTGFS);

	gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE,
			GPIO10 | GPIO11 | GPIO12);
	gpio_set_af(GPIOA, GPIO_AF10, GPIO10 | GPIO11 | GPIO12);

	my_usb_device = usbd_init(&otgfs_usb_driver, &dev, &config, usb_strings, 3, usbd_control_buffer, sizeof(usbd_control_buffer));
	usbd_register_set_config_callback(my_usb_device, hid_set_config);

	nvic_enable_irq(NVIC_OTG_FS_IRQ);
}

void usb_poll(void) {
	//usbd_poll(my_usb_device);
}

void
otg_fs_isr(void)
{
	usbd_poll(my_usb_device);
	gpio_toggle_led(LED4);
	//last_usb_request_time=system_millis;
}
