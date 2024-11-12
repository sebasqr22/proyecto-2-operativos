#include "biblioteca.h"
#include <stdio.h>
#include <string.h>

int fd;

int abrir_conexion() {
    // Abre la conexión con el Arduino
    fd = abrir_conexion();
    if (fd == -1) {
        return 1;  // Error al abrir la conexión
    }
}

int cerrar_conexion() {
    // Cierra la conexión con el Arduino
    cerrar_conexion(fd);
}

int enviar_comando(const char *comando) {
    // Envía un comando al Arduino
    if (enviar_datos(fd, comando, strlen(comando)) == -1) {
        printf("Error al enviar el comando al Arduino\n");
    } else {
        printf("Comando enviado: %s\n", comando);
    }
}