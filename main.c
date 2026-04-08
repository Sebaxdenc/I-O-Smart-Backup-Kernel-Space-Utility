#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "smart_copy.h"

// Función auxiliar para medir tiempo con alta precisión en segundos
double get_time_diff(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
}

int main(int argc, char *argv[]) {
    // Verificar que se proporcionaron los argumentos obligatorios
    if (argc != 3) {
        printf("==================================================\n");
        printf("   Smart Backup Kernel-Space Utility (Benchmark)  \n");
        printf("==================================================\n");
        fprintf(stderr, "Uso correcto:\n  %s <archivo_origen> <archivo_destino>\n\n", argv[0]);
        fprintf(stderr, "Ejemplo:\n  %s data_1GB.bin backup.bin\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *src = argv[1];
    const char *dest = argv[2];
    
    // Nombres temporales para que ambas pruebas generen algo único y se puedan comparar 
    char dest_sys[1024];
    char dest_stdio[1024];
    snprintf(dest_sys, sizeof(dest_sys), "%s.sys", dest);
    snprintf(dest_stdio, sizeof(dest_stdio), "%s.stdio", dest);

    struct timespec start, end;
    double time_sys, time_stdio;
    int res;

    printf("\n");
    printf("==================================================\n");
    printf("         SMART BACKUP (BENCHMARK) v1.0            \n");
    printf("==================================================\n");
    printf("- Origen      : %s\n", src);
    printf("- Destino Base: %s\n", dest);
    printf("- Buffer Size : %d bytes (1 Paginación Kernel)\n", BUFFER_SIZE);
    printf("--------------------------------------------------\n");

    // 1. Ejecutar y medir Syscalls (Kernel-Space Utility)
    printf("[1] Copiando usando System Calls directas...\n");
    clock_gettime(CLOCK_MONOTONIC, &start);
    res = sys_smart_copy(src, dest_sys);
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    if (res == SC_SUCCESS) {
        time_sys = get_time_diff(start, end);
        printf("    ✓ Éxito. Generado: %s\n", dest_sys);
        printf("    ✓ TIEMPO (Syscalls) : %.6f segundos\n", time_sys);
    } else {
        printf("    ✗ Error en función sys_smart_copy. Código de error: %d\n", res);
        time_sys = -1.0;
    }

    printf("--------------------------------------------------\n");

    // 2. Ejecutar y medir Standard I/O (User-Space Buffering)
    printf("[2] Copiando usando libreria estadar (stdio.h)...\n");
    clock_gettime(CLOCK_MONOTONIC, &start);
    res = stdio_smart_copy(src, dest_stdio);
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    if (res == SC_SUCCESS) {
        time_stdio = get_time_diff(start, end);
        printf("    ✓ Éxito. Generado: %s\n", dest_stdio);
        printf("    ✓ TIEMPO (Stdio)    : %.6f segundos\n", time_stdio);
    } else {
        printf("    ✗ Error en función stdio_smart_copy. Código de error: %d\n", res);
        time_stdio = -1.0;
    }

    printf("\n==================================================\n");
    printf("                 COMPARATIVA FINAL                \n");
    printf("==================================================\n");
    
    // Imprimir resultados sólo si ambas pruebas pasaron
    if (time_sys >= 0 && time_stdio >= 0) {
        printf("[Syscalls] Kernel-Space IO : %.6f s\n", time_sys);
        printf("[Stdio.h] User-Space Buff. : %.6f s\n", time_stdio);
        
        double diff = time_sys - time_stdio;
        if (diff > 0) {
            printf("\n-> VENDEDOR: Stdio (%.6f s más rápido).\n", diff);
            printf("\nExplicación Técnica:\n");
            printf("- Las operaciones estandar (fopen, fread) optimizan la copia debido\n");
            printf("- al manejo interno del 'Double Copy', alineación automática de stdio,\n");
            printf("- o un menor número de Context Switches delegados por libc.\n");
        } else {
            printf("\n-> GANADOR: Syscalls (%.6f s más rápido).\n", -diff);
            printf("\nExplicación Técnica:\n");
            printf("- Al forzar el BUFFER_SIZE a 4096 bytes (tamaño de página predeterminado),\n");
            printf("- logramos evitar Context Switches innecesarios mientras se eludió la doble\n");
            printf("- carga en memoria buffer que `stdio.h` aplica por defecto en user-space.\n");
        }
        
    } else {
        printf("No se pudo completar el benchmark debido a un error previo.\n");
    }
    printf("==================================================\n\n");

    return EXIT_SUCCESS;
}
