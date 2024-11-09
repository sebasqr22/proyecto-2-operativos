#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/usb.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>

#define VENDOR_ID 0x1A86
#define PRODUCT_ID 0x7523

// Estructura para el dispositivo USB CH341
struct ch341_usb {
    struct usb_device *usb_dev;
    struct usb_interface *usb_intf;
    struct tty_port tty_port;
    struct urb *urb_lectura;
    struct urb *urb_escritura;
};

// Configuración de la conexión USB
static const struct usb_device_id ch341_id_table[] = {
    { USB_DEVICE(VENDOR_ID, PRODUCT_ID) },
    {}
};
MODULE_DEVICE_TABLE(usb, ch341_id_table);

// Inicialización de la conexión
static int ch341_abrir(struct tty_struct *tty, struct file *file) {
    struct ch341_usb *ch341 = tty->driver_data;

    ch341->urb_lectura = usb_alloc_urb(0, GFP_KERNEL);
    if (!ch341->urb_lectura)
        return -ENOMEM;

    ch341->urb_escritura = usb_alloc_urb(0, GFP_KERNEL);
    if (!ch341->urb_escritura) {
        usb_free_urb(ch341->urb_lectura);
        return -ENOMEM;
    }

    return 0; // Exito
}

// Cerrado de conexión
static void ch341_cerrar(struct tty_struct *tty, struct file *file) {
    struct ch341_usb *ch341 = tty->driver_data;

    usb_free_urb(ch341->urb_lectura);
    usb_free_urb(ch341->urb_escritura);
}

// Función de escritura de datos
static int ch341_escribir(struct tty_struct *tty, const unsigned char *buffer, int len) {
    struct ch341_usb *ch341 = tty->driver_data;
    int bytes_trans;

    bytes_trans = usb_bulk_msg(ch341->usb_dev,
                               usb_sndbulkpipe(ch341->usb_dev, 0x02),
                               (void *)buffer, len, &len, 1000);

    return bytes_trans ? bytes_trans : len;
}

// Lectura de datos
static void ch341_lectura_cb(struct urb *urb) {
    struct ch341_usb *ch341 = urb->context;
    struct tty_port *tty_port = &ch341->tty_port;

    tty_insert_flip_string(tty_port, urb->transfer_buffer, urb->actual_length);
    tty_flip_buffer_push(tty_port);

    usb_submit_urb(urb, GFP_KERNEL); // Enviar el URB nuevamente
}

// Configuración inicial del dispositivo
static int ch341_probe(struct usb_interface *intf, const struct usb_device_id *id) {
    struct ch341_usb *ch341;

    ch341 = kzalloc(sizeof(*ch341), GFP_KERNEL);
    if (!ch341)
        return -ENOMEM;

    ch341->usb_dev = usb_get_dev(interface_to_usbdev(intf));
    ch341->usb_intf = intf;
    usb_set_intfdata(intf, ch341);

    return 0;
}

// Desconexión del dispositivo
static void ch341_disconnect(struct usb_interface *intf) {
    struct ch341_usb *ch341 = usb_get_intfdata(intf);

    usb_set_intfdata(intf, NULL);
    usb_put_dev(ch341->usb_dev);
    kfree(ch341);
}

// Definir el controlador USB
static struct usb_driver ch341_driver = {
    .name = "ch341",
    .probe = ch341_probe,
    .disconnect = ch341_disconnect,
    .id_table = ch341_id_table,
};

module_usb_driver(ch341_driver);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Desarrollador Original");
MODULE_DESCRIPTION("Driver USB para CH341");
