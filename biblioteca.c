#include "biblioteca.h"
#include <fcntl.h>       // Para open
#include <unistd.h>      // Para close, read, write
#include <stdio.h>       // Para perror

// Define la ruta al dispositivo USB (esto puede cambiar dependiendo del sistema)
#define DEVICE_PATH "/dev/ttyUSB0"  // Ajusta esto a tu dispositivo

// Abre el dispositivo USB y devuelve el file descriptor
int abrir_driver() {
    int fd = open(DEVICE_PATH, O_RDWR);
    if (fd == -1) {
        perror("Error al abrir el dispositivo USB");
    }
    return fd;
}

// Cierra el dispositivo USB
void cerrar_driver(int fd) {
    if (close(fd) == -1) {
        perror("Error al cerrar el dispositivo USB");
    }
}

// Escribe datos al dispositivo USB
int escribir_datos(int fd, const char *buffer, size_t len) {
    int bytes_written = write(fd, buffer, len);
    if (bytes_written == -1) {
        perror("Error al escribir en el dispositivo USB");
    }
    return bytes_written;
}

// Lee datos desde el dispositivo USB
int leer_datos(int fd, char *buffer, size_t len) {
    int bytes_read = read(fd, buffer, len);
    if (bytes_read == -1) {
        perror("Error al leer desde el dispositivo USB");
    }
    return bytes_read;
}