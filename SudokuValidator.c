#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <sys/syscall.h>

int sudoku[9][9];

int columnas_validas = 1;

int verificar_fila(int fila) {
    int visto[10] = {0};

    for (int j = 0; j < 9; j++) {
        int num = sudoku[fila][j];

        if (num < 1 || num > 9 || visto[num])
            return 0;

        visto[num] = 1;
    }
    return 1;
}

int verificar_columna(int col) {
    int visto[10] = {0};

    for (int i = 0; i < 9; i++) {
        int num = sudoku[i][col];

        if (num < 1 || num > 9 || visto[num])
            return 0;

        visto[num] = 1;
    }
    return 1;
}

int verificar_subcuadro(int fila_inicio, int col_inicio) {
    int visto[10] = {0};

    for (int i = fila_inicio; i < fila_inicio + 3; i++) {
        for (int j = col_inicio; j < col_inicio + 3; j++) {
            int num = sudoku[i][j];

            if (num < 1 || num > 9 || visto[num])
                return 0;

            visto[num] = 1;
        }
    }
    return 1;
}

void *verificar_columnas_thread(void *arg) {

    long tid = syscall(SYS_gettid);

    printf("El thread que ejecuta la revision de columnas es: %ld\n", tid);

    for (int i = 0; i < 9; i++) {
        printf("En la revision de columnas el siguiente es un thread en ejecucion: %ld\n", tid);

        if (!verificar_columna(i)) {
            columnas_validas = 0;
            pthread_exit(0);
        }
    }

    pthread_exit(0);
}

void ejecutar_ps(pid_t pid_padre) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("Error en fork");
        return;
    }

    if (pid == 0) {
        char pid_str[20];
        sprintf(pid_str, "%d", pid_padre);

        printf("\nEjecutando: ps -p %s -lLf\n\n", pid_str);

        execlp("ps", "ps", "-p", pid_str, "-lLf", NULL);

        perror("Error en execlp");
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
        perror("Error al abrir archivo");
        return 1;
    }

    char *data = mmap(NULL, 81, PROT_READ, MAP_PRIVATE, fd, 0);
    if (data == MAP_FAILED) {
        perror("Error en mmap");
        close(fd);
        return 1;
    }

    for (int i = 0; i < 81; i++) {
        sudoku[i / 9][i % 9] = data[i] - '0';
    }

    pid_t pid_padre = getpid();

    ejecutar_ps(pid_padre);

    pthread_t hilo_columnas;

    pthread_create(&hilo_columnas, NULL, verificar_columnas_thread, NULL);
    pthread_join(hilo_columnas, NULL);

    printf("El thread en el que se ejecuta main es: %ld\n", syscall(SYS_gettid));

    int filas_validas = 1;

    for (int i = 0; i < 9; i++) {

        printf("Revisando fila %d en thread principal...\n", i);

        if (!verificar_fila(i)) {
            filas_validas = 0;
            break;
        }
    }

    int subcuadros_validos = 1;

    for (int i = 0; i < 9; i += 3) {
        for (int j = 0; j < 9; j += 3) {
            if (!verificar_subcuadro(i, j)) {
                subcuadros_validos = 0;
                break;
            }
        }
    }

    if (columnas_validas && filas_validas && subcuadros_validos) {
        printf("Sudoku Resuelto\n");
    } else {
        printf("Sudoku inválido\n");
    }

    ejecutar_ps(pid_padre);

    munmap(data, 81);
    close(fd);

    return 0;
}