#include "smart_copy.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

/**
 * sys_smart_copy: Implementación de la copia usando llamadas al sistema (Syscalls) puras.
 * 
 * Usa open(), read(), write() y close(). 
 * El buffer está definido (4096 bytes o 4KB). Al usar llamadas directas al kernel, si 
 * el buffer fuera pequeño (ej. 1 byte) habría un "Context Switch" en cada operación.
 * Configurando esto en exactamente una página (4KB), balanceamos el overhead del switch
 * entre modo kernel/modo usuario maximizando el throughput y sin gastar RAM excesiva.
 */
int sys_smart_copy(const char *src, const char *dest) {
    // Abrir archivo origen en modo solo lectura
    int fd_src = open(src, O_RDONLY);
    if (fd_src < 0) {
        perror("Error al abrir archivo de origen (SysCall)");
        return SC_ERR_OPEN_SRC;
    }

    struct stat st;
    // Obtenemos información para mantener permisos
    if (fstat(fd_src, &st) < 0) {
        perror("Error obteniendo stats del origen (SysCall)");
        close(fd_src);
        return SC_ERR_OPEN_SRC;
    }

    // Abrir archivo destino en modo escritura, crear si no existe o sobreescribir.
    int fd_dest = open(dest, O_WRONLY | O_CREAT | O_TRUNC, st.st_mode);
    if (fd_dest < 0) {
        perror("Error al abrir/crear archivo de destino (SysCall)");
        close(fd_src);
        return SC_ERR_OPEN_DEST;
    }

    char *buffer = (char *)malloc(BUFFER_SIZE);
    if (!buffer) {
        perror("Error asignando memoria (SysCall)");
        close(fd_src);
        close(fd_dest);
        return SC_ERR_NO_MEM;
    }

    ssize_t bytes_read, bytes_written;
    
    // Leemos eficientemente usando un ciclo de bytes
    while ((bytes_read = read(fd_src, buffer, BUFFER_SIZE)) > 0) {
        char *ptr = buffer;
        ssize_t remaining = bytes_read;
        
        while (remaining > 0) {
            bytes_written = write(fd_dest, ptr, remaining);
            
            if (bytes_written < 0) {
                // Manejo de errores específico
                if (errno == ENOSPC) {
                    fprintf(stderr, "Error SysCall: No hay espacio libre en disco (ENOSPC).\n");
                } else if (errno == EACCES) {
                    fprintf(stderr, "Error SysCall: Permisos insuficientes (EACCES).\n");
                } else {
                    perror("Error escribiendo archivo de destino (SysCall)");
                }
                free(buffer);
                close(fd_src);
                close(fd_dest);
                return SC_ERR_WRITE;
            }
            ptr += bytes_written;
            remaining -= bytes_written;
        }
    }

    if (bytes_read < 0) {
        perror("Error leyendo archivo de origen (SysCall)");
        free(buffer);
        close(fd_src);
        close(fd_dest);
        return SC_ERR_READ;
    }

    // Limpieza estricta de recursos
    free(buffer);
    close(fd_src);
    close(fd_dest);
    
    return SC_SUCCESS;
}

/**
 * stdio_smart_copy: Implementación usando la librería estándar de C (stdio.h).
 * 
 * Función: Usa fopen(), fread(), fwrite() y fclose().
 * Diferencia de rendimiento: 'stdio.h' usa una abstracción sobre las Syscalls llamada
 * "User-space buffering". Agrupa varias peticiones pequeñas de lectura/escritura en una 
 * zona de memoria administrada por la librería en "Modo usuario". Posteriormente invoca 
 * al sistema operativo (Context Switch) todo junto.
 * Con operaciones de más de 4KB (nuestro uso de BUFFER_SIZE), stdio añade cierta 
 * sobrecarga "doble copiado" (kernel space -> stdio buffer -> user buffer), o resulta  
 * muy similar a usar las Syscalls con buffer grande directamente.
 */
int stdio_smart_copy(const char *src, const char *dest) {
    FILE *f_src = fopen(src, "rb");
    if (!f_src) {
        perror("Error al abrir archivo de origen (Stdio)");
        return SC_ERR_OPEN_SRC;
    }

    FILE *f_dest = fopen(dest, "wb");
    if (!f_dest) {
        perror("Error al abrir/crear archivo de destino (Stdio)");
        fclose(f_src);
        return SC_ERR_OPEN_DEST;
    }

    char *buffer = (char *)malloc(BUFFER_SIZE);
    if (!buffer) {
        perror("Error asignando memoria (Stdio)");
        fclose(f_src);
        fclose(f_dest);
        return SC_ERR_NO_MEM;
    }

    size_t bytes_read, bytes_written;
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, f_src)) > 0) {
        bytes_written = fwrite(buffer, 1, bytes_read, f_dest);
        if (bytes_written != bytes_read) {
            // Manejo de errores específico
            if (errno == ENOSPC) {
                fprintf(stderr, "Error Stdio: No hay espacio libre en disco (ENOSPC).\n");
            } else {
                perror("Error escribiendo archivo de destino (Stdio)");
            }
            free(buffer);
            fclose(f_src);
            fclose(f_dest);
            return SC_ERR_WRITE;
        }
    }

    // ferror verifica el flag de error del stream
    if (ferror(f_src)) {
        perror("Error leyendo archivo de origen (Stdio)");
        free(buffer);
        fclose(f_src);
        fclose(f_dest);
        return SC_ERR_READ;
    }

    // Limpieza estricta de recursos
    free(buffer);
    fclose(f_src);
    fclose(f_dest);
    
    return SC_SUCCESS;
}
