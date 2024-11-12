#include "biblioteca.h"
#include <stdio.h>

int main() {
    int fd = abrir_driver();
    if (fd == -1) {
        return 1;  // Error al abrir el driver
    }

    // Ejemplo de escritura
    const char *mensaje = "Hola, dispositivo!";
    if (escribir_datos(fd, mensaje, sizeof(mensaje)) == -1) {
        printf("Error escribiendo datos\n");
    }

    // Ejemplo de lectura
    char buffer[256];
    int bytes_leidos = leer_datos(fd, buffer, sizeof(buffer));
    if (bytes_leidos > 0) {
        printf("Datos recibidos: %s\n", buffer);
    }

    cerrar_driver(fd);
    return 0;
}
