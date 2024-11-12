#ifndef BIBLIOTECA_H
#define BIBLIOTECA_H

#include <stddef.h>  // Para size_t

// Funciones de la biblioteca
int abrir_driver();
void cerrar_driver(int fd);
int escribir_datos(int fd, const char *buffer, size_t len);
int leer_datos(int fd, char *buffer, size_t len);

#endif // BIBLIOTECA_H
