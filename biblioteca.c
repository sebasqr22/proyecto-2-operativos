#include "biblioteca.h"
#include <fcntl.h>       // Para open
#include <unistd.h>      // Para close, read, write
#include <sys/ioctl.h>   // Para ioctl
#include <stdio.h>       // Para perror

// Abre el driver de dispositivo y devuelve el file descriptor
int abrir_driver() {
    int fd = open("/dev/tu_driver", O_RDWR);
    if (fd == -1) {
        perror("Error al abrir el driver");
    }
    return fd;
}

// Cierra el file descriptor del driver
void cerrar_driver(int fd) {
    if (close(fd) == -1) {
        perror("Error al cerrar el driver");
    }
}

// Mueve el dispositivo hacia la derecha
int mover_derecha(int fd) {
    if (ioctl(fd, COMANDO_MOVER_DERECHA) == -1) {
        perror("Error en mover_derecha");
        return -1;
    }
    return 0;
}

// Mueve el dispositivo hacia la izquierda
int mover_izquierda(int fd) {
    if (ioctl(fd, COMANDO_MOVER_IZQUIERDA) == -1) {
        perror("Error en mover_izquierda");
        return -1;
    }
    return 0;
}

// Lee datos desde el dispositivo
int leer(int fd, char *buffer, size_t size) {
    int bytes_read = read(fd, buffer, size);
    if (bytes_read == -1) {
        perror("Error en lectura");
    }
    return bytes_read;
}

// Escribe datos al dispositivo
int escribir(int fd, const char *data, size_t size) {
    int bytes_written = write(fd, data, size);
    if (bytes_written == -1) {
        perror("Error en escritura");
    }
    return bytes_written;
}
