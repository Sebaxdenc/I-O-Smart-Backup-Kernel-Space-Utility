CC = gcc
CFLAGS = -Wall -Wextra -g -O2
TARGET = smart_backup
SRC = main.c backup_engine.c
OBJ = $(SRC:.c=.o)
HEADERS = smart_copy.h

# Regla por defecto (compila el programa principal)
all: $(TARGET)

# Cómo construir el ejecutable
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Regla para limpiar los archivos generados y los de prueba
clean:
	rm -f $(TARGET) $(OBJ)
	rm -rf test_1kb.bin test_1mb.bin test_1gb.bin
	rm -rf test_1kb.bin.* test_1mb.bin.* test_1gb.bin.*

# Regla para generar archivos de prueba (1KB, 1MB)
gen_tests:
	@echo "Generando archivo de 1KB..."
	dd if=/dev/urandom of=test_1kb.bin bs=1024 count=1
	@echo "Generando archivo de 1MB..."
	dd if=/dev/urandom of=test_1mb.bin bs=1024 count=1024
	@echo "Para 1GB ejecuta (toma tiempo): dd if=/dev/urandom of=test_1gb.bin bs=1M count=1024"

test_all: all gen_tests
	@echo "\n--- [PRUEBA 1KB] ---"
	./$(TARGET) test_1kb.bin backup_1kb.bin
	@echo "\n--- [PRUEBA 1MB] ---"
	./$(TARGET) test_1mb.bin backup_1mb.bin

.PHONY: all clean gen_tests test_all
