#include "biblioteca.h"
#include <fcntl.h>       // Para open
#include <unistd.h>      // Para close, write
#include <stdio.h>       // Para perror

#define DEVICE_PATH "/dev/ttyUSB0" 

// Abre la conexión USB con el Arduino y devuelve el file descriptor
int abrir_conexion() {
    int fd = open(DEVICE_PATH, O_RDWR | O_NOCTTY | O_SYNC);
    if (fd == -1) {
        perror("Error al abrir la conexión con Arduino");
    }
    return fd;
}

// Cierra la conexión USB con el Arduino
void cerrar_conexion(int fd) {
    if (close(fd) == -1) {
        perror("Error al cerrar la conexión con Arduino");
    }
}

int enviar_comando(int fd, const char *comando) {
    int bytes_written = write(fd, comando, strlen(comando));
    if (bytes_written == -1) {
        perror("Error al enviar comando al Arduino");
    }
    return bytes_written;
}

// Envía datos al Arduino por USB
int enviar_datos(int fd, const char *buffer, size_t len) {
    int bytes_written = write(fd, buffer, len);
    if (bytes_written == -1) {
        perror("Error al enviar datos al Arduino");
    }
    return bytes_written;
}
