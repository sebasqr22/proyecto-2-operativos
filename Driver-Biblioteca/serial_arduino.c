#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <termios.h>
#include "biblioteca.h" // Incluir el encabezado

// Descriptor global para el puerto serial
static int serial_port = -1;

// Implementación de la función para enviar un mensaje al Arduino
int sendToArduino(const char* message) {
    const char* port_name = "/dev/ttyACM0"; // Cambia esto al puerto serial correcto

    // Abrir el puerto serial si no está abierto
    if (serial_port < 0) {
        serial_port = open(port_name, O_RDWR);
    }

    if (serial_port < 0) {
        fprintf(stderr, "Error al abrir el puerto serial: %s\n", strerror(errno));
        return 1;
    }

    // Buffer para almacenar la respuesta
    char received[255] = {0};
    char user_input[10];

    // Solicitar entrada del usuario antes de enviar el mensaje
    printf("Press any key to continue ");
    fgets(user_input, sizeof(user_input), stdin);

    // Escribir el mensaje al puerto serial
    ssize_t write_result = write(serial_port, message, strlen(message));
    if (write_result < 0) {
        fprintf(stderr, "Error al escribir en el puerto serial: %s\n", strerror(errno));
        return 1;
    } else {
        printf("Enviado: %s\n", message);
    }

    // Leer la respuesta del puerto serial
    ssize_t read_result = read(serial_port, received, sizeof(received) - 1);
    if (read_result < 0) {
        fprintf(stderr, "Error al leer del puerto serial: %s\n", strerror(errno));
        return 1;
    } else {
        received[read_result] = '\0'; // Asegurarse de que la cadena esté terminada
        printf("Leído: %s\n", received);
    }

    return 0;
}
