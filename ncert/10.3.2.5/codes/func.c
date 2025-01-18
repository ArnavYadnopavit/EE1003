#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#define ORDER 2

// Define a struct for a matrix
typedef struct matrix {
    double mat[ORDER][ORDER];
} matrix;

typedef struct vector{
    double vec[ORDER];
}vector;
// Function to perform LU decomposition
void luDecompose(matrix A, matrix *L, matrix *U) {
    int i, j, k;

    // Initialize L and U
    for (i = 0; i < ORDER; i++) {
        for (j = 0; j < ORDER; j++) {
            if (i == j) {
                L->mat[i][j] = 1; // Diagonal elements of L are 1
            } else {
                L->mat[i][j] = 0;
            }
            U->mat[i][j] = 0;
        }
    }

    // Perform decomposition
    for (i = 0; i < ORDER; i++) {
        // Calculate U
        for (j = i; j < ORDER; j++) {
            U->mat[i][j] = A.mat[i][j];
            for (k = 0; k < i; k++) {
                U->mat[i][j] -= L->mat[i][k] * U->mat[k][j];
            }
        }

        // Calculate L
        for (j = i + 1; j < ORDER; j++) {
            L->mat[j][i] = A.mat[j][i];
            for (k = 0; k < i; k++) {
                L->mat[j][i] -= L->mat[j][k] * U->mat[k][i];
            }
            L->mat[j][i] /= U->mat[i][i];
        }
    }
}

// Function to print a matrix
void printMatrix(matrix M, const char *name) {
    printf("%s:\n", name);
    for (int i = 0; i < ORDER; i++) {
        for (int j = 0; j < ORDER; j++) {
            printf("%8.4f ", M.mat[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

vector solve(matrix A, vector b){
	matrix L,U;
	for (int i = 0; i < ORDER; i++) {
        for (int j = 0; j < ORDER; j++) {
            L.mat[i][j] = 0;  // Diagonal elements of L are 1
            U.mat[i][j] = 0;                // All elements of U are 0
        }
    }
	luDecompose(A,&L,&U);
	vector y={0,0},x={0,0};
	y.vec[0]=b.vec[0];
	y.vec[1]=b.vec[1]-L.mat[1][0]*y.vec[0];
	x.vec[1]=y.vec[1]/U.mat[1][1];
	x.vec[0]=(y.vec[0]-x.vec[1]*U.mat[0][1])/U.mat[0][0];
	return x;

} 


