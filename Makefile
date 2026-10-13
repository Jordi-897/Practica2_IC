# GNU Makefile para compilar `cuatroEnraya9.0.c`

CC      ?= gcc
TARGET  := cuatroEnraya
SRC     := cuatroEnraya9.0.c

CFLAGS_BASE := -Wall -Wextra -std=c99 -pedantic

.PHONY: all debug release optimize native sanitize clean run help

# Objetivo por defecto: build optimizado normal (-O2)
all: release

# Build de depuración (-g -O0)
debug: CFLAGS := $(CFLAGS_BASE) -g -O0 -DDEBUG
debug: $(TARGET)

# Build de publicación (-O2)
release: CFLAGS := $(CFLAGS_BASE) -O2 -DNDEBUG
release: $(TARGET)

# Build con optimización máxima (-O3 + march native + LTO)
optimize: CFLAGS := $(CFLAGS_BASE) -O3 -march=native -mtune=native -flto -DNDEBUG
optimize: LDFLAGS := -flto
optimize: $(TARGET)

# Build tuned al hardware local (-O3 + march/tune native, LTO, vectorización)
native: CFLAGS := $(CFLAGS_BASE) -O3 -march=native -mtune=native -flto -funroll-loops -fomit-frame-pointer -DNDEBUG
native: LDFLAGS := -flto
native: $(TARGET)

# Build con AddressSanitizer/UBSan (depuración de memoria)
sanitize: CFLAGS := $(CFLAGS_BASE) -g -O1 -fsanitize=address,undefined -fno-omit-frame-pointer -DDEBUG
sanitize: LDFLAGS := -fsanitize=address,undefined
sanitize: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

# Ejecuta el juego (compila si es necesario)
run: all
	./$(TARGET)

# Limpia artefactos de compilación
clean:
	rm -f $(TARGET) *.o

# Ayuda con los objetivos disponibles
help:
	@echo "Objetivos disponibles:"
	@echo "  make / make all      Compila en modo release (-O2) [por defecto]"
	@echo "  make debug           Compila con símbolos de depuración (-g -O0)"
	@echo "  make release         Compila optimizado (-O2)"
	@echo "  make optimize        Compila con -O3 + march native + LTO"
	@echo "  make native          Compila tuned al hardware (+unroll/vectorización)"
	@echo "  make sanitize        Compila con AddressSanitizer + UBSan"
	@echo "  make run             Compila (release) y ejecuta el juego"
	@echo "  make clean           Elimina el ejecutable y objetos"
	@echo "  make help            Muestra esta ayuda"
	@echo ""
	@echo "Variables:"
	@echo "  CC=gcc               Cambia el compilador (ej. make CC=clang)"
