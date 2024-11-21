#include <linux/init.h>
#include <linux/module.h>
#include <linux/usb.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/slab.h>

#define ACM_VID  0x2a03  // Vendor ID del Arduino
#define ACM_PID  0x0043  // Product ID del Arduino

struct acm_device {
    struct usb_device *udev;
    struct usb_interface *interface;
    unsigned char *bulk_in_buffer;
    size_t bulk_in_size;
    __u8 bulk_in_endpointAddr;
    __u8 bulk_out_endpointAddr;
    struct tty_port tty_port;  // Estructura para el subsistema TTY
};

static struct tty_driver *acm_tty_driver;

// Función de apertura de TTY
static int acm_open(struct tty_struct *tty, struct file *file) {
    struct acm_device *dev = tty->driver_data;

    if (!dev) {
        return -ENODEV;
    }

    tty->driver_data = dev;
    dev_info(&dev->interface->dev, "TTY abierto.\n");

    return 0;
}

// Función de cierre de TTY
static void acm_close(struct tty_struct *tty, struct file *file) {
    struct acm_device *dev = tty->driver_data;

    if (dev) {
        dev_info(&dev->interface->dev, "TTY cerrado.\n");
    }
}

// Operaciones del TTY
static const struct tty_operations acm_tty_ops = {
    .open = acm_open,
    .close = acm_close,
};

// Función para manejar el dispositivo USB
static int acm_probe(struct usb_interface *interface, const struct usb_device_id *id) {
    struct usb_device *udev = interface_to_usbdev(interface);
    struct acm_device *dev;
    struct usb_host_interface *iface_desc;
    struct usb_endpoint_descriptor *endpoint;
    int i;

    dev = kzalloc(sizeof(*dev), GFP_KERNEL);
    if (!dev) {
        dev_err(&interface->dev, "No se pudo asignar memoria.\n");
        return -ENOMEM;
    }

    dev->udev = usb_get_dev(udev);
    dev->interface = interface;
    tty_port_init(&dev->tty_port);  // Inicializa la estructura TTY

    iface_desc = interface->cur_altsetting;

    // Identificar endpoints
    for (i = 0; i < iface_desc->desc.bNumEndpoints; i++) {
        endpoint = &iface_desc->endpoint[i].desc;

        if (usb_endpoint_is_bulk_in(endpoint)) {
            dev->bulk_in_endpointAddr = endpoint->bEndpointAddress;
            dev->bulk_in_size = usb_endpoint_maxp(endpoint);
            dev->bulk_in_buffer = kmalloc(dev->bulk_in_size, GFP_KERNEL);
            if (!dev->bulk_in_buffer) {
                dev_err(&interface->dev, "No se pudo asignar buffer IN.\n");
                goto error;
            }
        } else if (usb_endpoint_is_bulk_out(endpoint)) {
            dev->bulk_out_endpointAddr = endpoint->bEndpointAddress;
        }
    }

    if (!(dev->bulk_in_endpointAddr && dev->bulk_out_endpointAddr)) {
        dev_err(&interface->dev, "Endpoints requeridos no encontrados.\n");
        goto error;
    }

    usb_set_intfdata(interface, dev);

    // Asignar el dispositivo al subsistema TTY
    tty_port_register_device(&dev->tty_port, acm_tty_driver, 0, &interface->dev);
    dev_info(&interface->dev, "ACM device conectado y registrado en TTY.\n");
    return 0;

error:
    if (dev->bulk_in_buffer)
        kfree(dev->bulk_in_buffer);
    kfree(dev);
    return -ENODEV;
}

// Función de desconexión del dispositivo USB
static void acm_disconnect(struct usb_interface *interface) {
    struct acm_device *dev = usb_get_intfdata(interface);

    usb_set_intfdata(interface, NULL);
    if (dev) {
        tty_unregister_device(acm_tty_driver, 0);
        tty_port_destroy(&dev->tty_port);

        if (dev->bulk_in_buffer)
            kfree(dev->bulk_in_buffer);
        usb_put_dev(dev->udev);
        kfree(dev);
    }

    dev_info(&interface->dev, "ACM device desconectado.\n");
}

// Tabla de dispositivos soportados
static const struct usb_device_id acm_table[] = {
    { USB_DEVICE(ACM_VID, ACM_PID) },
    {}  // Terminador
};
MODULE_DEVICE_TABLE(usb, acm_table);

// Registro del driver USB
static struct usb_driver acm_driver = {
    .name = "driver_ACM",
    .probe = acm_probe,
    .disconnect = acm_disconnect,
    .id_table = acm_table,
};

// Inicialización del módulo
static int __init acm_init(void) {
    int result;
// Registrar el driver TTY
    acm_tty_driver = tty_alloc_driver(1, TTY_DRIVER_RESET_TERMIOS);
    if (!acm_tty_driver) {
        pr_err("No se pudo registrar el driver TTY.\n");
        return -ENOMEM;
    }

    acm_tty_driver->driver_name = "driver_ACM";
    acm_tty_driver->name = "ttyACM";
    acm_tty_driver->major = 0;  // Asigna dinámicamente
    acm_tty_driver->type = TTY_DRIVER_TYPE_SERIAL;
    acm_tty_driver->subtype = SERIAL_TYPE_NORMAL;
    acm_tty_driver->init_termios = tty_std_termios;
    acm_tty_driver->flags = TTY_DRIVER_REAL_RAW;
    tty_set_operations(acm_tty_driver, &acm_tty_ops);

    result = tty_register_driver(acm_tty_driver);
    if (result) {
        pr_err("No se pudo registrar el driver TTY: %d\n", result);
        return result;
    }

    // Registrar el driver USB
    result = usb_register(&acm_driver);
    if (result) {
        tty_unregister_driver(acm_tty_driver);
        return result;
    }

    pr_info("Driver ACM inicializado.\n");
    return 0;
}

// Finalización del módulo
static void __exit acm_exit(void) {
    usb_deregister(&acm_driver);
    tty_unregister_driver(acm_tty_driver);
    pr_info("Driver ACM finalizado.\n");
}

module_init(acm_init);
module_exit(acm_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SEBAS");
MODULE_DESCRIPTION("Driver USB ACM para dispositivos tipo serie.");
