#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#define ORDER 2

// Define a struct for a matrix
typedef struct matrix {
    double mat[ORDER][ORDER];
} matrix;

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

// Main function
int main() {
    matrix A = {{{1, -1}, {3, -3}}}; // Input matrix
    matrix L, U;

    // Perform LU decomposition
    luDecompose(A, &L, &U);

    // Print results
    printMatrix(A, "Matrix A");
    printMatrix(L, "Lower Triangular Matrix L");
    printMatrix(U, "Upper Triangular Matrix U");

    return 0;
}

