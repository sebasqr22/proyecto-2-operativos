#ifndef BIBLIOTECA_H
#define BIBLIOTECA_H

#include <stddef.h>  // Para size_t

// Funciones de la biblioteca
int abrir_conexion();
void cerrar_conexion(int fd);
int enviar_datos(int fd, const char *buffer, size_t len);


// Comandos especificos


#endif // BIBLIOTECA_H
