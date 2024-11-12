#ifndef BIBLIOTECA_H
#define BIBLIOTECA_H

#include <stddef.h>  // Para size_t

// Define los comandos de control
#define IOC_MAGIC 'k'               // Número mágico único para el driver
#define COMANDO_MOVER_DERECHA _IO(IOC_MAGIC, 1)
#define COMANDO_MOVER_IZQUIERDA _IO(IOC_MAGIC, 2)

// Declaración de las funciones de la biblioteca
int abrir_driver();
void cerrar_driver(int fd);
int mover_derecha(int fd);
int mover_izquierda(int fd);
int leer(int fd, char *buffer, size_t size);
int escribir(int fd, const char *data, size_t size);

#endif // BIBLIOTECA_H
