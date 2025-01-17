#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#define ORDER 2
typedef struct matrix{
	double mat[ORDER][ORDER];
}matrix;

void row_reduction(matrix* m) {
    for (int i = 0; i < ORDER; i++) {
        if (m->mat[i][i] == 0) {
            for (int k = i + 1; k < ORDER; k++) {
                if (m->mat[k][i] != 0) {
                    for (int j = 0; j < ORDER; j++) {
                        double temp = m->mat[i][j];
                        m->mat[i][j] = m->mat[k][j];
                        m->mat[k][j] = temp;
                    }
                    break;
                }
            }
        }
        for (int k = i + 1; k < ORDER; k++) {
            double factor = m->mat[k][i] / m->mat[i][i];
            for (int j = 0; j < ORDER; j++) {
                m->mat[k][j] -= factor * m->mat[i][j];
            }
        }
    }
}

int is_singular(matrix m) {
    row_reduction(&m);
    for (int i = 0; i < ORDER; i++) {
        int all_zero = 1;
        for (int j = 0; j < ORDER; j++) {
            if (m.mat[i][j] != 0) {
                all_zero = 0;
                break;
            }
        }
        if (all_zero) {
            return 1;
        }
    }
    return 0;
}




