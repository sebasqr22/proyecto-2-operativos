#include "biblioteca.h"
#include <stdio.h>
#include <string.h>

int main() {
    // Abre la conexión con el Arduino
    int fd = abrir_conexion();
    if (fd == -1) {
        return 1;  // Error al abrir la conexión
    }

    const char *comando = "ENCENDER_LED\n";  // Envía un comando al Arduino
    if (enviar_datos(fd, comando, strlen(comando)) == -1) {
        printf("Error al enviar el comando al Arduino\n");
    } else {
        printf("Comando enviado: %s\n", comando);
    }

    // Cierra la conexión con el Arduino
    cerrar_conexion(fd);
    return 0;
}
