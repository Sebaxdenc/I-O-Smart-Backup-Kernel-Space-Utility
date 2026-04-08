# Proyecto de Parcial: Smart Backup Kernel-Space Utility

## Descripción
Este proyecto es una utilidad de copia de seguridad (backup) desarrollada en C que compara el rendimiento de dos estrategias de copiado de archivos:
1. **Llamadas al Sistema (Kernel-Space Syscalls):** Utiliza operaciones directas del kernel (`open()`, `read()`, `write()`, `close()`) con un tamaño de búfer alineado a la página de memoria estándar (4KB / 4096 bytes) para mitigar el costo del Context Switch.
2. **Biblioteca Estándar de C (User-Space Buffering):** Utiliza `fopen()`, `fread()`, `fwrite()`, `fclose()`, delegando el buffering a nivel de usuario provisto por `stdio.h`.

El objetivo principal es medir el tiempo en precisión a nano-segundos para diferentes tamaños de archivo (1KB, 1MB, y 1GB) y comprender el impacto detrás del *User-space buffering* vs. el *Context Switch overhead*.

## Requisitos de Entorno
* Sistema Operativo: GNU/Linux, MacOS, o Windows con WSL (Windows Subsystem for Linux). Las syscalls POSIX empleadas no funcionan de manera nativa en el Command Prompt o terminal estándar de Windows.
* Dependencias:
  * `gcc`: Para compilar el código.
  * `make`: Para la construcción automatizada.

## Instrucciones Paso a Paso

### 1. Compilar el programa
Desde una terminal (en WSL o Linux) orientada a la carpeta del proyecto, ejecuta:
```bash
make all
```
Esto creará el archivo ejecutable `smart_backup`.

### 2. Ejecutar Pruebas Automatizadas (1KB y 1MB)
El `Makefile` incluye una rutina especial que genera archivos binarios aleatorios (con la utilidad `dd`), corre el programa en ellos comparando ambas técnicas, e imprime la comparación.
```bash
make test_all
```

### 3. Ejecutar Manualmente (Opcional - p.ej. para pruebas custom)
Para usar el programa para hacer la copia de un archivo cualquiera, la sintaxis correcta es:
```bash
./smart_backup <archivo_origen> <archivo_destino>
```
Ejemplo con el archivo de 1GB:
1. Generar tu archivo de 1GB (tarda un poco):
   ```bash
   dd if=/dev/urandom of=test_1gb.bin bs=1M count=1024
   ```
2. Ejecutar la comparativa en ese archivo:
   ```bash
   ./smart_backup test_1gb.bin backup_1gb.bin
   ```

### 4. Limpieza del directorio
Para borrar el programa compilado, los binarios temporales de prueba, y todos los backups generados, basta con correr:
```bash
make clean
```
