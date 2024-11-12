# Makefile para compilar el módulo del kernel, la biblioteca y el adaptador

# Archivos de módulo y biblioteca
obj-m += driver.o
BIBLIOTECA_OBJ = biblioteca.o
BIBLIOTECA_LIB = libbiblioteca.a

# Directorios
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

# Objetivo principal
all: modulo biblioteca adaptador

# Compilar el módulo del kernel
modulo:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

# Compilar la biblioteca
biblioteca: $(BIBLIOTECA_OBJ)
	ar rcs $(BIBLIOTECA_LIB) $(BIBLIOTECA_OBJ)

# Compilar el archivo biblioteca.o
biblioteca.o: biblioteca.c biblioteca.h
	gcc -c biblioteca.c -o biblioteca.o

# Compilar el adaptador y enlazar la biblioteca
adaptador: adaptador_biblioteca.c $(BIBLIOTECA_LIB)
	gcc adaptador_biblioteca.c -L. -lb
