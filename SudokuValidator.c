#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <sys/syscall.h>
#include <omp.h>

int sudoku[9][9];

int columnas_validas = 1;

int verificar_fila(int fila) {
    int visto[10] = {0};

    for (int j = 0; j < 9; j++) {
        int num = sudoku[fila][j];
        if (num < 1 || num > 9 || visto[num]) return 0;
        visto[num] = 1;
    }
    return 1;
}

int verificar_columna(int col) {
    int visto[10] = {0};

    for (int i = 0; i < 9; i++) {
        int num = sudoku[i][col];
        if (num < 1 || num > 9 || visto[num]) return 0;
        visto[num] = 1;
    }
    return 1;
}

int verificar_subcuadro(int fila_inicio, int col_inicio) {
    int visto[10] = {0};

    for (int i = fila_inicio; i < fila_inicio + 3; i++) {
        for (int j = col_inicio; j < col_inicio + 3; j++) {
            int num = sudoku[i][j];
            if (num < 1 || num > 9 || visto[num]) return 0;
            visto[num] = 1;
        }
    }
    return 1;
}

void *verificar_columnas_thread(void *arg) {

    long tid = syscall(SYS_gettid);
    printf("Thread columnas (pthread). TID: %ld\n", tid);

    for (int i = 0; i < 9; i++) {
        printf("Revisando columna %d en thread %ld\n", i, tid);

        if (!verificar_columna(i)) {
            columnas_validas = 0;
            pthread_exit(0);
        }
    }

    pthread_exit(0);
}

void ejecutar_ps(pid_t pid_padre) {
    pid_t pid = fork();

    if (pid == 0) {
        char pid_str[20];
        sprintf(pid_str, "%d", pid_padre);

        printf("\nEjecutando: ps -p %s -lLf\n\n", pid_str);

        execlp("ps", "ps", "-p", pid_str, "-lLf", NULL);
        perror("execlp");
        exit(1);
    } else {
        wait(NULL);
    }
}

int main(int argc, char *argv[]) {

    if (argc < 2) {
        printf("Uso: %s archivo\n", argv[0]);
        return 1;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    char *data = mmap(NULL, 81, PROT_READ, MAP_PRIVATE, fd, 0);
    if (data == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }

    for (int i = 0; i < 81; i++) {
        sudoku[i / 9][i % 9] = data[i] - '0';
    }

    pid_t pid_padre = getpid();

    ejecutar_ps(pid_padre);

    pthread_t hilo;
    pthread_create(&hilo, NULL, verificar_columnas_thread, NULL);
    pthread_join(hilo, NULL);

    printf("Thread main TID: %ld\n", syscall(SYS_gettid));

    int filas_validas = 1;

    omp_set_num_threads(9);
    #pragma omp parallel for reduction(&&:filas_validas)
    for (int i = 0; i < 9; i++) {
        printf("Thread %d revisando fila %d\n", omp_get_thread_num(), i);
        filas_validas = filas_validas && verificar_fila(i);
    }

    int subcuadros_validos = 1;

    omp_set_num_threads(9);
    #pragma omp parallel for collapse(2) schedule(dynamic) reduction(&&:subcuadros_validos)
    for (int i = 0; i < 9; i += 3) {
        for (int j = 0; j < 9; j += 3) {
            printf("Thread %d revisando subcuadro (%d,%d)\n",
                   omp_get_thread_num(), i, j);

            subcuadros_validos =
                subcuadros_validos && verificar_subcuadro(i, j);
        }
    }

    if (columnas_validas && filas_validas && subcuadros_validos) {
        printf("\nSudoku Resuelto\n");
    } else {
        printf("\nSudoku Inválido\n");
    }

    ejecutar_ps(pid_padre);

    munmap(data, 81);
    close(fd);

    return 0;
}