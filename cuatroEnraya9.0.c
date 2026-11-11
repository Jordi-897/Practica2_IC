#include<stdio.h>
#include<stdbool.h>
#include<stdlib.h>
#include<unistd.h>
#include<time.h>
#include<limits.h>
#include<mpi.h>

#define F 600
#define C 10

/*
 * max / min
 * ---------
 * Funciones auxiliares para obtener el máximo o mínimo entre dos enteros.
 */
int max(int n1, int n2){
    int res = n1;
    if(n1<n2){
        res = n2;
    }
    return res;
}

int min(int n1, int n2){
    int res=n1;
    if(n1>n2){
        res = n2;
    }
    return res;
}

/*
 * rectangulos
 * -----------
 * Detecta patrones rectangulares 2x3 del mismo jugador.
 * Se usa como heurística para evaluar posiciones favorables.
 *
 * jugador = 1 → 'o'
 * jugador = -1 → 'x'
 */
int rectangulos(const int tablero[][C],bool jug){  
    int res=0;
    int jugador = jug ? 1:-1;
#ifdef _OPENMP
    #pragma omp parallel for reduction(+:res)
#endif
    for(int i=0;i<F-1;i++){
        for(int j=0;j<C-2;j++){
            if(
                tablero[i][j] == jugador &&
                tablero[i][j+1] == jugador &&
                tablero[i][j+2] == jugador &&
                tablero[i+1][j] == jugador &&
                tablero[i+1][j+1] == jugador &&
                tablero[i+1][j+2] == jugador
            ) res += 1;
        }
    }

    return res;
}

/*
 * diagonales_peligrosas
 * ----------------------
 * Detecta diagonales casi completas (3 fichas + 1 hueco).
 * 'par' alterna filas para evaluar patrones específicos.
 * Optimizado con fusión de bucles por fila para mejorar la localidad de caché
 * y permitir vectorización sobre tableros grandes (F x C).
 */
int diagonales_peligrosas(int tablero[][C],bool jug,bool par){
    int jugador = jug ? 1:-1;
    int res=0;

#ifdef _OPENMP
    #pragma omp parallel for reduction(+:res)
#endif
    for(int i= par ? 0:1;i<F-3;i += 2){
        // Diagonales ↘
        for(int j=0;j<C-3;j++){
            if(
                (tablero[i][j] == 0 &&
                tablero[i+1][j+1] == jugador &&
                tablero[i+2][j+2] == jugador &&
                tablero[i+3][j+3] == jugador) ||
                (tablero[i][j] == jugador &&
                tablero[i+1][j+1] == jugador &&
                tablero[i+2][j+2] == 0 &&
                tablero[i+3][j+3] == jugador)
            ) res++;
        }

        // Diagonales ↙
        for(int j=C-1;j>=3;j--){
            if(
                (tablero[i][j] == 0 &&
                tablero[i+1][j-1] == jugador &&
                tablero[i+2][j-2] == jugador &&
                tablero[i+3][j-3] == jugador) ||
                (tablero[i][j] == jugador &&
                tablero[i+1][j-1] == jugador &&
                tablero[i+2][j-2] == 0 &&
                tablero[i+3][j-3] == jugador)
            ) res++;
        }
    }

    return res;
}

/*
 * tresEnRayaHF
 * ------------
 * Detecta patrones horizontales peligrosos (3 fichas + hueco).
 * 'par' alterna filas para evaluar patrones específicos.
 */
int tresEnRayaHF(const int tablero[][C],bool jug,bool par){
    int res = 0;
    int jugador = jug ? 1:-1;

    for(int i = par ? 0:1;i<F;i+=2){
        for(int j=0;j+3<C;j++){
            if(
                (tablero[i][j] == jugador &&
                tablero[i][j+1] == 0 &&
                tablero[i][j+2] == jugador &&
                tablero[i][j+3] == jugador) ||
                (tablero[i][j] == 0 &&
                tablero[i][j+1] == jugador &&
                tablero[i][j+2] == jugador &&
                tablero[i][j+3] == jugador) ||
                (tablero[i][j] == jugador &&
                tablero[i][j+1] == jugador &&
                tablero[i][j+2] == 0 &&
                tablero[i][j+3] == jugador) ||
                (tablero[i][j] == jugador &&
                tablero[i][j+1] == jugador &&
                tablero[i][j+2] == jugador &&
                tablero[i][j+3] == 0)
            ) res = (res+1)*(j+1);
        }
    }

    return res;
}

/*
 * PosiciónFavorable_El_7 / El_7_Inverso
 * -------------------------------------
 * Detecta patrones en forma de "7" y "7 invertido".
 * Son heurísticas avanzadas para evaluar posiciones fuertes.
 */
int PosicionFavorable_El_7_Inverso(const int tablero[][C],bool jug){
    int res=0;
    int jugador = jug ? 1:-1;
#ifdef _OPENMP
    #pragma omp parallel for reduction(+:res)
#endif
    for(int i=F-1;i>=2;i--){
        for(int j=0;j<C-2;j++){
            if(tablero[i-2][j] == jugador &&
                tablero[i-2][j+1] == jugador &&
                tablero[i-2][j+2] == jugador &&
                tablero[i-1][j+1] == jugador &&
                tablero[i][j+2] == jugador
            ) res += 1;
        }
    }
    return res;
}

int PosicionFavorable_El_7(const int tablero[][C],bool jug){
    int res=0;
    int jugador = jug ? 1:-1;
#ifdef _OPENMP
    #pragma omp parallel for reduction(+:res)
#endif
    for(int i=F-1;i>=2;i--){
        for(int j=0;j<C-2;j++){
            if(tablero[i][j] == jugador &&
                tablero[i-1][j+1] == jugador &&
                tablero[i-2][j] == jugador &&
                tablero[i-2][j+1] == jugador &&
                tablero[i-2][j+2] == jugador
            ) res += 1;
        }
    }
    return res;
}

/*
 * tresEnRaya
 * ----------
 * Detecta patrones de 3 en raya horizontales, verticales y diagonales.
 * Se usa como parte de la heurística del bot.
 * Optimizado mediante fusión de los 7 bucles en un único recorrido por filas
 * para reutilizar las filas en caché L1/L2 y facilitar el paralelismo de datos (SIMD).
 */
int tresEnRaya(int tablero[][C], bool jugado){
    int res = 0;
    int jugador = jugado ? 1:-1;

#ifdef _OPENMP
    #pragma omp parallel for reduction(+:res)
#endif
    for(int i=0;i<F;i++){
        // Horizontal (ambos patrones fusionados en una sola pasada de fila)
        for(int j=0;j+3<C;j++){
            if(tablero[i][j+1] == jugador && tablero[i][j+2] == jugador){
                if(tablero[i][j] == jugador && tablero[i][j+3] == 0) res++;
                if(tablero[i][j] == 0 && tablero[i][j+3] == jugador) res++;
            }
        }

        // Vertical
        if(i+2<F){
            for(int j=0;j<C;j++){
                if(
                    tablero[i][j] == jugador &&
                    tablero[i+1][j] == jugador &&
                    tablero[i+2][j] == jugador
                ) res++;
            }
        }

        // Diagonales ↘ y ↙ (fusionadas para reutilizar las filas i..i+3 en caché)
        if(i+3<F){
            for(int j=0;j+3<C;j++){
                if(tablero[i+1][j+1] == jugador && tablero[i+2][j+2] == jugador){
                    if(tablero[i][j] == jugador && tablero[i+2][j+3] == 0) res++;
                    if(tablero[i][j] == 0 && tablero[i+3][j+3] == jugador) res++;
                }
            }
            for(int j=C-1;j-3>=0;j--){
                if(tablero[i+1][j-1] == jugador && tablero[i+2][j-2] == jugador){
                    if(tablero[i][j] == jugador && tablero[i+3][j-3] == 0) res++;
                    if(tablero[i][j] == 0 && tablero[i+3][j-3] == jugador) res++;
                }
            }
        }
    }

    return res;
}

/*
 * lleno
 * -----
 * Comprueba si el tablero está completamente lleno.
 */
bool lleno(int tablero[][C]){
    bool cero = true;

    for(int i=0;i<F;i++){
        for(int j=0;j<C;j++){
            if(tablero[i][j]==0){
                cero = false;
            }
        }
    }

    return cero;
}

/*
 * comprobacionH / comprobacionV
 * ------------------------------
 * Comprueba si hay 4 en raya horizontales o verticales.
 * Devuelve:
 *   1 → gana jugador 'o'
 *  -1 → gana jugador 'x'
 *   0 → nadie gana
 */
int comprobacionH(int tablero[][C]){
    int j1=0;
    int j2=0;
    int res=0;
    for(int i=0;i<F;i++){
        j2=0;
        j1=0;
        for(int j=0;j<C;j++){
            if(tablero[i][j]==1){
                j2 = 0;
                j1 += 1;
            } else if(tablero[i][j]==-1){
                j1 = 0;
                j2 += 1;
            } else {
                j2 = 0;
                j1 = 0;
            }

            if(j2 >= 4){
                res = -1;
            }else if(j1 >=4){
                res = 1;
            }
        }
    }

    return res;
}

int comprobacionV(int tablero[][C]){
    int j1=0;
    int j2=0;
    int res=0;
    for(int i=0;i<C;i++){
        j2=0;
        j1=0;
        for(int j=0;j<F;j++){
            if(tablero[j][i]==1){
                j2 = 0;
                j1 += 1;
            } else if(tablero[j][i]==-1){
                j1 = 0;
                j2 += 1;
            } else {
                j2 = 0;
                j1 = 0;
            }

            if(j2 >= 4){
                res = -1;
            }else if(j1 >=4){
                res = 1;
            }
        }
    }

    return res;
}

/*
 * ganador
 * -------
 * Comprueba si hay un ganador:
 *   - Horizontal
 *   - Vertical
 *   - Diagonales ↘ y ↙
 */
int ganador(int tablero[][C]){
    int res = 0;
    res = comprobacionH(tablero);
    if(res == 0){
        res = comprobacionV(tablero);
    }
    if(res==0){
        for(int i=0;i<F;i++){
            for(int j=0;j<C;j++){
                if(i+3 < F && j+3 < C){
                    if(tablero[i][j]==1 && tablero[i+1][j+1]==1 && tablero[i+2][j+2]==1 && tablero[i+3][j+3]==1){
                        res = 1;
                    } else if(tablero[i][j]==-1 && tablero[i+1][j+1]==-1 && tablero[i+2][j+2]==-1 && tablero[i+3][j+3]==-1){
                        res = -1;
                    }
                }
                if(i+3 < F && j-3 > -1){
                    if(tablero[i][j]==1 && tablero[i+1][j-1]==1 && tablero[i+2][j-2]==1 && tablero[i+3][j-3]==1){
                        res = 1;
                    }else if(tablero[i][j]==-1 && tablero[i+1][j-1]==-1 && tablero[i+2][j-2]==-1 && tablero[i+3][j-3]==-1){
                        res = -1;
                    }
                }

            }
        }
    }

    return res;
}

/*
 * mostrar
 * -------
 * Imprime el tablero con colores ANSI.
 */
void mostrar(int tablero[][C]){
    system("clear");
    for(int i=0;i<F;i++){
        printf("\033[31;91m| \033[0m");
        for(int j=0;j<C;j++){
            switch (tablero[i][j]){
                case 0:
                    printf("-");
                    break;
                case 1:
                    printf("\033[32;36mo\033[0m");
                    break;
                case -1:
                    printf("\033[32;31mx\033[0m");
                    break;
            }
            printf("\033[31;91m | \033[0m");
        }
        printf("\n\033[31;93m-");
        for(int j=0;j<C;j++){
            printf("----");
        }
        printf("\033[0m\n");
    }
}

/*
 * animacion
 * ---------
 * Simula la caída de una ficha animada.
 */
void animacion(int tablero[][C], int movimiento,bool ficha, int profundidad){
    int jugador = (ficha) ? 1:-1;
    if(profundidad < F && !(tablero[profundidad][movimiento]!=0)){
        tablero[profundidad][movimiento] = jugador;
        mostrar(tablero);
        tablero[profundidad][movimiento] = 0;
        sleep(1);
        animacion(tablero, movimiento, ficha, profundidad+1);
    }
}

/*
 * movPosi
 * -------
 * Comprueba si una columna es válida para colocar ficha.
 */
bool movPosi(int tablero[][C], int movimiento){
    bool res = false;
    if(!(movimiento<0 || movimiento > C-1)){
        if(tablero[0][movimiento]==0){
            res = true;
        }
    }
    return res;
}

/*
 * revovinar
 * ---------
 * Elimina la última ficha colocada en una columna (undo).
 */
void revovinar(int tablero[][C], int movimiento){
    for(int i=0;i<F;i++){
        if(tablero[i][movimiento] != 0){
            tablero[i][movimiento] =0;
            break;
        }
    }
}

/*
 * mover
 * -----
 * Coloca una ficha en la columna indicada.
 * jugador = true → 'o'
 * jugador = false → 'x'
 */
void mover(int tablero[][C], int movimiento,bool jugador){
    for(int i=1;i<=F;i++){
        if(tablero[F-i][movimiento]==0){
            if(jugador){
                tablero[F-i][movimiento] = 1;
            } else {
                tablero[F-i][movimiento] = -1;
            }

            break;
        }
    }
}

/*
 * moverJ
 * ------
 * Movimiento del jugador humano.
 * Pide columna por consola.
 */
void moverJ(int tablero[][C], bool jugador){
    int movimiento;
    do{
        printf("Introduzca un numero del 1 al %d\n", C);
        scanf("%d",&movimiento);
        movimiento = movimiento-1;
        if(!(movPosi(tablero,movimiento))){
            printf("Error no es posible mover ahi\n");
        }
    }while(!(movPosi(tablero, movimiento)));
    mover(tablero, movimiento, jugador);
}

/*
 * inicializar
 * -----------
 * Pone todo el tablero a 0.
 */
void inicializar(int tablero[][C]){
    for(int i=0; i<F;i++){
        for(int j=0;j<C;j++){
            tablero[i][j] = 0;
        }
    }
}

/*
 * bot (Minimax + Alfa-Beta)
 * -------------------------
 * IA que evalúa el tablero mediante:
 *   - Minimax con poda alfa-beta
 *   - Heurísticas avanzadas:
 *       * tresEnRaya
 *       * rectangulos
 *       * diagonales_peligrosas
 *       * patrones en forma de 7
 *
 * umbral → profundidad máxima
 */
int bot(int tablero[][C], bool jugador, int umbral,int alfa,int beta){
    int res = 0;
    int puntuacion = 0;
    int estadoGanador = ganador(tablero);

    // Si hay ganador → devolver puntuación extrema
    if(estadoGanador!=0){
        if(estadoGanador==-1){
            res = 10000*umbral;      // Máquina gana → gran valor positivo
        }else{
            res = -10000*umbral;     // Jugador gana → gran valor negativo
        }
    } else {
        bool estaLleno = lleno(tablero);
        if(estaLleno || umbral==0){
            // Tablero lleno o profundidad agotada → heurística
            if(estaLleno){
                res = 0;                 // Empate
            } else {
                // Heurística avanzada basada en múltiples patrones
                res += tresEnRaya(tablero,false);
                res -= tresEnRaya(tablero,true);
                res += rectangulos(tablero,false);
                res -= rectangulos(tablero,true);
                res += PosicionFavorable_El_7(tablero,false)*5;
                res -= PosicionFavorable_El_7(tablero,true)*5;
                res += PosicionFavorable_El_7_Inverso(tablero,false)*5;
                res -= PosicionFavorable_El_7_Inverso(tablero,true)*5;
                res += tresEnRayaHF(tablero,false,true)*12;
                res -= tresEnRayaHF(tablero,true,false)*12;
                res -= diagonales_peligrosas(tablero,true,false)*20;
                res += diagonales_peligrosas(tablero,false,true)*20;
            }
        } else {
        // Turno de la máquina (MAX)
        if(!(jugador)){
            int mejorPuntuacion = -999999;
            for(int j = 0;j<C;j++){
                if(movPosi(tablero,j)){
                    mover(tablero, j, jugador);                 // Simula movimiento
                    puntuacion = bot(tablero, true,umbral-1,alfa,beta);
                    revovinar(tablero, j);                      // Deshace movimiento
                    mejorPuntuacion = max(mejorPuntuacion, puntuacion);
                    alfa = max(alfa,puntuacion);

                    // Poda alfa-beta
                    if(alfa >= beta) {
                        return mejorPuntuacion;
                    };
                }
            }
            res = mejorPuntuacion;

        // Turno del jugador (MIN)
        } else {
            int mejorPuntuacion = 999999;
            for(int j = 0;j<C;j++){
                if(movPosi(tablero,j)){
                    mover(tablero, j, jugador);
                    puntuacion = bot(tablero, false, umbral-1,alfa,beta);
                    revovinar(tablero, j);
                    mejorPuntuacion = min(mejorPuntuacion, puntuacion);
                    beta = min(beta,puntuacion);

                    // Poda alfa-beta
                    if(alfa >= beta) {
                        return mejorPuntuacion;
                    };
                }
            }
            res = mejorPuntuacion;
        }
        }
    }
    
    return res;
}

/*
 * best_movimiento (Paralelizado con MPI)
 * --------------------------------------
 * Calcula el mejor movimiento para la máquina usando Minimax + Alfa-Beta
 * repartiendo las columnas entre los procesos MPI disponibles.
 *
 * Funcionamiento:
 *   - Cada proceso evalúa un subconjunto de columnas (reparto cíclico)
 *     sobre su propia copia del tablero llamando a bot().
 *   - Mediante MPI_Reduce (con MPI_MAX) se reúnen las puntuaciones de las
 *     7 columnas en el proceso maestro (rank 0).
 *   - El proceso maestro selecciona la columna con mayor puntuación
 *     (manteniendo el factor aleatorio determinista con rand()) y difunde
 *     la columna elegida al resto de procesos con MPI_Bcast.
 */
void best_movimiento(int tablero[][C], int umbral, int rank, int num_procs){
    int mejorPuntuacion = -999999;
    int movimiento = 0;
    int puntuacionesLocal[C];
    int puntuacionesGlobal[C];

    // Inicializa vectores de puntuaciones con un valor mínimo centinela
    for(int j = 0; j < C; j++){
        puntuacionesLocal[j] = INT_MIN;
        puntuacionesGlobal[j] = INT_MIN;
    }

    // Reparto cíclico de las columnas entre los procesos MPI
    for(int j = rank; j < C; j += num_procs){
        if(movPosi(tablero, j)){
            mover(tablero, j, false);                                           // Simula movimiento máquina
            puntuacionesLocal[j] = bot(tablero, true, umbral, -999999, 999999); // Evalúa rama
            revovinar(tablero, j);                                              // Deshace
        }
    }

    // Reducción: se combinan las puntuaciones de todas las columnas en rank 0
    MPI_Reduce(puntuacionesLocal, puntuacionesGlobal, C, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);

    if(rank == 0){
        // Inicializa movimiento con la última columna válida
        for(int i = 0; i < C; i++){
            if (movPosi(tablero, i)){
                movimiento = i;
            }
        }

        // El maestro selecciona el mejor movimiento a partir de los resultados reducidos
        for(int j = 0; j < C; j++){
            if(movPosi(tablero, j)){
                int Puntuacion = puntuacionesGlobal[j];

                if(Puntuacion > mejorPuntuacion){
                    mejorPuntuacion = Puntuacion;
                    movimiento = j;
                }

                // Factor aleatorio para evitar patrones repetitivos
                movimiento = (Puntuacion == mejorPuntuacion && rand()%4==0) ? j : movimiento;

                //printf("Puntucacion: %d, Movimiento: %d\n",Puntuacion,j+1);
            }
        }
    }

    // El maestro difunde la columna elegida para mantener sincronizados todos los tableros
    MPI_Bcast(&movimiento, 1, MPI_INT, 0, MPI_COMM_WORLD);

    mover(tablero, movimiento, false);  // Ejecuta movimiento final en todos los procesos
}

/*
 * main
 * ----
 * Control principal del juego (versión MPI):
 *   - Inicializa el entorno MPI y el tablero en todos los procesos
 *   - Ejecuta la secuencia predefinida de movimientos del jugador
 *   - En el turno de la máquina, todos los procesos colaboran en best_movimiento()
 *   - Sólo el proceso maestro (rank 0) muestra el tablero e imprime por pantalla
 */
int main(int argc, char *argv[]) {
    int rank, num_procs;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    int tablero[F][C];
    int turnos = 0;
    int umbral = 9;
    int i = 0;

    int movimientos[F * C];
    int num_movimientos = F * C;
    for (int k = 0; k < num_movimientos; k++) {
        movimientos[k] = (k % C) + 1;
    }

    inicializar(tablero);
    srand(999);

    MPI_Barrier(MPI_COMM_WORLD);
    double start = MPI_Wtime();

    while (ganador(tablero) == 0 && !lleno(tablero)) {

        // Si no quedan movimientos predefinidos, gana la máquina
        if (i >= num_movimientos) {
            if (rank == 0) {
                printf("Se han agotado los movimientos predefinidos.\n");
                printf("Gana la maquina.\n");
            }
            break;
        }

        if (rank == 0) {
            mostrar(tablero);
        }

        // Intentar ejecutar el siguiente movimiento predefinido
        int movimiento = movimientos[i] - 1;
        i++;

        if (!movPosi(tablero, movimiento)) {
            if (rank == 0) {
                printf("Movimiento %d no valido. Se pasa al siguiente.\n",
                       movimiento + 1);
            }
            continue;
        }

        mover(tablero, movimiento, true);

        // Comprobar si la partida ha terminado
        if (ganador(tablero) != 0 || lleno(tablero)) {
            break;
        }

        // Turno de la máquina
        if (rank == 0) {
            printf("La maquina esta pensando...\n");
            printf("Umbral de pensamiento: %d\n", umbral);
        }

        best_movimiento(tablero, umbral, rank, num_procs);
        turnos++;

        // Ajuste dinámico de profundidad
        umbral = (C * F / 2) == turnos
                     ? 1000
                     : umbral + (int)(!(bool)(turnos % 3));
    }

    MPI_Barrier(MPI_COMM_WORLD);
    double final = MPI_Wtime();

    if (rank == 0) {
        mostrar(tablero);

        // Resultado final
        if (i >= num_movimientos &&
            ganador(tablero) == 0 &&
            !lleno(tablero)) {
            printf("Han ganado las x (la maquina).\n");
        } else {
            switch (ganador(tablero)) {
                case 0:
                    printf("Empate\n");
                    break;
                case 1:
                    printf("Han ganado las o\n");
                    break;
                case -1:
                    printf("Han ganado las x\n");
                    break;
            }
        }

        printf("Tiempo: %f\n", final - start);
    }

    MPI_Finalize();

    return 0;
}
