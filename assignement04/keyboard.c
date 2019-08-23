#include <linux/module.h>	/* Needed by all modules */
#include <linux/kernel.h>	/* Needed for KERN_INFO */
#include <linux/usb.h>
#include <linux/hid.h>

MODULE_LICENSE("Dual MIT/GPL");
MODULE_DESCRIPTION("hello World module");

static int keyboard_connect(struct usb_interface *intf, const struct usb_device_id *id)
{
	printk(KERN_INFO "Hello, Keyboard\n");
	return 0;
}

static void keyboard_disconnect(struct usb_interface *intf)
{
	printk(KERN_INFO "Keyboard device disconnected\n");
}

static const struct usb_device_id usb_keyboard_table[] = { 
	{ USB_INTERFACE_INFO(USB_INTERFACE_CLASS_HID, USB_INTERFACE_SUBCLASS_BOOT, USB_INTERFACE_PROTOCOL_KEYBOARD) },
	{ },
};

MODULE_DEVICE_TABLE(usb, usb_keyboard_table);

static struct usb_driver usb_driver = {
	.name = "hello_world_driver",
	.id_table = usb_keyboard_table,
	.probe = keyboard_connect,
	.disconnect = keyboard_disconnect,
};

int init_module(void)
{
	if(usb_register(&usb_driver) >= 0)
		printk(KERN_INFO "Hello world !\n");
	return 0;
}
void cleanup_module(void)
{
	printk(KERN_INFO "Goodbye world.\n");
	usb_deregister(&usb_driver);
}
