# 4 en Raya — Práctica de Ingeniería Informática

Juego de **Connect 4 (4 en raya)** en consola implementado en C. El tablero es de
6 filas × 7 columnas. El jugador humano juega con `o` y la máquina (cuando se
elige ese modo) con `x`.

La IA se basa en **Minimax con poda alfa-beta** combinado con heurísticas
avanzadas (tres en raya, rectángulos 2×3, diagonales peligrosas, patrones en
forma de "7", etc.) y una profundidad de búsqueda que se ajusta dinámicamente.

---

## Estructura del proyecto

```
Practica2_IC/
├── cuatroEnraya9.0.c   # Código fuente del juego
├── Makefile            # Reglas de compilación con distintos flags de optimización
└── README.md           # Este archivo
```

## Requisitos

- Compilador C compatible con C99 (gcc o clang).
- `make` (GNU Make).
- Terminal con soporte ANSI para los colores.

## Compilación

El `Makefile` ofrece varios objetivos según el nivel de optimización/diagnóstico
que se quiera:

| Objetivo     | Flags principales                                | Uso típico                          |
|--------------|--------------------------------------------------|-------------------------------------|
| `make` / `all` / `release` | `-Wall -Wextra -O2 -DNDEBUG` | Ejecución normal (por defecto) |
| `debug`      | `-g -O0 -DDEBUG`                                 | Depuración con gdb/lldb             |
| `optimize`   | `-O3 -march=native -mtune=native -flto`         | Máxima optimización portable-nativa |
| `native`     | `-O3 -march=native … -funroll-loops`            | Tuned al hardware local             |
| `sanitize`   | `-O1 -fsanitize=address,undefined`               | Detección de errores de memoria     |
| `run`        | compila `release` y ejecuta                      | Jugar directamente                  |
| `clean`      | —                                                | Elimina binarios generados          |
| `help`       | —                                                | Muestra la ayuda de `make`          |

### Ejemplos

```bash
# Compilar en modo release (por defecto)
make

# Compilar con máxima optimización
make optimize

# Compilar tuned al hardware (recomendado para jugar contra la máquina)
make native

# Compilar con símbolos de depuración
make debug

# Compilar y ejecutar directamente
make run

# Limpiar artefactos
make clean

# Cambiar compilador
make CC=clang
```

## Ejecución

Tras compilar se genera el ejecutable `cuatroEnraya`:

```bash
./cuatroEnraya
```

Al arrancar, el programa pregunta el modo de juego:

```
Introduzca 0 si quiere jugar contra otro jugador
Introduzca 1 si quiere jugar contra la maquina
```

- **Modo 0:** dos jugadores humanos se turnan introduciendo columnas (1–7).
- **Modo 1:** el jugador humano mueve con `o` y la máquina responde con `x`
  usando Minimax. La profundidad de búsqueda se incrementa al avanzar la partida.

Durante la partida se muestra el tablero con colores ANSI y una animación de
caída de cada ficha. Al finalizar se indica el resultado (ganan las `o`,
ganan las `x` o empate) y el tiempo total empleado.

## Notas

- El programa usa `system("clear")`, por lo que se recomienda ejecutarlo en un
  terminal Linux/macOS (o WSL/Git Bash en Windows).
- La IA puede tardar varios segundos por movimiento en profundidades altas; usa
  `make native` para aprovechar el hardware local.
