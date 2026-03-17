#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

int sudoku[9][9];

int verificar_fila(int fila){
    int visto[10] = {0};

    for (int j = 0; j<9; j++){
        int num = sudoku[fila][j];

        if (num < 1 || num > 9 || visto[num] == 1)
        return 0;

        visto[num] = 1;
    }
    return 1;
};

int verificar_columna(int columna){
    int visto[10] = {0};

    for (int i = 0; i<9; i++){
        int num = sudoku[i][columna];

        if (num < 1 || num > 9 || visto[num] == 1)
        return 0;

        visto[num] = 1;
    }
    return 1;
};