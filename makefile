# Nombre del módulo
obj-m += ch341.o

# Ruta al kernel. Puede que necesites ajustarla dependiendo de tu distribución.
KDIR := /lib/modules/$(shell uname -r)/build

# Ruta al directorio actual
PWD := $(shell pwd)

# Regla para compilar el módulo
all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

# Limpieza de archivos generados
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
