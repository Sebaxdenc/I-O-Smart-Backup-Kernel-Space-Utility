#ifndef SMART_COPY_H
#define SMART_COPY_H

#include <stddef.h>

/**
 * Definición del tamaño del buffer.
 * Usar exactamente 4096 bytes (tamaño de página estándar en muchas arquitecturas)
 * ayuda a maximizar el throughput al estar alineado con la memoria del SO y
 * minimizar la cantidad de Context Switches de las System Calls.
 */
#define BUFFER_SIZE 4096

// Valores de retorno para manejo de errores
#define SC_SUCCESS 0
#define SC_ERR_OPEN_SRC -1
#define SC_ERR_OPEN_DEST -2
#define SC_ERR_READ -3
#define SC_ERR_WRITE -4
#define SC_ERR_NO_MEM -5

// Prototipos de funciones
int sys_smart_copy(const char *src, const char *dest);
int stdio_smart_copy(const char *src, const char *dest);

#endif // SMART_COPY_H
