#include <stdio.h>
#include "biblioteca.h" // Incluir el encabezado con la función
#include <string.h>

// Función para enviar un mensaje al Arduino
void enviarMensaje(const char* mensaje) {
    char mensaje_nuevo[256];
    snprintf(mensaje_nuevo, sizeof(mensaje_nuevo), "%s\n", mensaje);

    if (sendToArduino(mensaje_nuevo) != 0) {
        fprintf(stderr, "Error al enviar el mensaje al Arduino.\n");
    } else {
        printf("Mensaje enviado correctamente: %s\n", mensaje_nuevo);
    }
}
