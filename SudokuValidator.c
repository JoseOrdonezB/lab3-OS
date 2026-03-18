#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

int sudoku[9][9];

int verificar_fila(int fila) {
    int visto[10] = {0};

    for (int j = 0; j < 9; j++) {
        int num = sudoku[fila][j];

        if (num < 1 || num > 9 || visto[num] == 1)
            return 0;

        visto[num] = 1;
    }

    return 1;
}

int verificar_columna(int col) {
    int visto[10] = {0};

    for (int i = 0; i < 9; i++) {
        int num = sudoku[i][col];

        if (num < 1 || num > 9 || visto[num] == 1)
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

            if (num < 1 || num > 9 || visto[num] == 1)
                return 0;

            visto[num] = 1;
        }
    }

    return 1;
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
        return 1;
    }

    for (int i = 0; i < 81; i++) {
        sudoku[i / 9][i % 9] = data[i] - '0';
    }

    for (int i = 0; i < 9; i++) {
        if (!verificar_fila(i) || !verificar_columna(i)) {
            printf("Sudoku inválido ❌\n");
            return 0;
        }
    }

    for (int i = 0; i < 9; i += 3) {
        for (int j = 0; j < 9; j += 3) {
            if (!verificar_subcuadro(i, j)) {
                printf("Sudoku inválido ❌\n");
                return 0;
            }
        }
    }

    printf("Sudoku válido ✅\n");

    munmap(data, 81);
    close(fd);

    return 0;
}