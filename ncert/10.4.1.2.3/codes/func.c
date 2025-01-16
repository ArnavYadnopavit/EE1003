#include <stdio.h>
#include <math.h>
#include <complex.h>
#include <stdlib.h>
//TO DO LIST
//QR DECOMPOSITION DONE
//QR Algo DONE
//MATRIX SUB DONE
//MATRIX SCALARMULT DONE
//MATRIX MULT DONE
//WILKINSON SHIFT DONE
//VECTOR NORM DONE
#define MAX_ITER 10000
#define ORDER 2
typedef struct matrix{
	double complex mat[ORDER][ORDER];
}matrix;
typedef struct QR{
	matrix Q;
	matrix R;
}QR;
double complex VectorInnerProduct(double complex* vector1,double complex* vector2){
	double complex ip=0;
	for (int i=0;i<ORDER;i++){
                ip+=vector1[i]*conj(vector2[i]);
        }
	return ip;
}
double complex VectorNorm(double complex* vector){
	return csqrt(VectorInnerProduct(vector,vector));
}
matrix Identity(){
	matrix Id;
	
	for (int i=0;i<ORDER;i++){
		for(int j=0;j<ORDER;j++){
			if (i==j){
				Id.mat[i][j]=1;
			}
			else{
				Id.mat[i][j]=0;
			}
		}
	}
	return Id;
}

matrix MatScalMult(matrix mat,double complex scal){
	matrix result;
	for (int i=0;i<ORDER;i++){
		for (int j=0;j<ORDER;j++){
			result.mat[i][j]=mat.mat[i][j]*scal;
		}
	}
	return result;
}
matrix MatSub(matrix mat1,matrix mat2){
	matrix result;

    for (int i=0;i<ORDER;i++){
            for (int j=0;j<ORDER;j++){
                result.mat[i][j]=mat1.mat[i][j]-mat2.mat[i][j];
            }       
    }       
        return result;
}
matrix MatMult(matrix mat1,matrix mat2) {
    matrix result;
    for (int i=0;i<ORDER;i++){
        for (int j=0;j<ORDER;j++){
            result.mat[i][j]=0;
        }
    }
    for (int i=0;i<ORDER;i++){
        for (int j=0;j<ORDER;j++){
            for (int k=0;k<ORDER;k++){
                result.mat[i][j]+=mat1.mat[i][k]*mat2.mat[k][j];
            }
        }
    }
    
    return result;
}
matrix trans(matrix mat){
    matrix result;
    for (int i=0;i<ORDER;i++){
        for (int j=0;j<ORDER;j++){
        result.mat[i][j]=conj(mat.mat[j][i]);
        }
    }
    return result;
}
double complex WilkinsonShift(matrix A){
    double complex delta = (A.mat[ORDER-2][ORDER-2]-A.mat[ORDER-1][ORDER-1])/2;
    if (cabs(delta)==0 && cabs(A.mat[ORDER-1][ORDER-2])==0){
        return A.mat[ORDER-1][ORDER-1]+1;
    }
    else if (cabs(delta)==0){
        return A.mat[ORDER-1][ORDER-1]-A.mat[ORDER-1][ORDER-2]+1;
    }
    else if(cabs(A.mat[ORDER-1][ORDER-2])==0){
        return A.mat[ORDER-1][ORDER-1]+1;
    }
    else{
        return A.mat[ORDER-1][ORDER-1]-((delta/cabs(delta))*(A.mat[ORDER-1][ORDER-2]*A.mat[ORDER-1][ORDER-2])/(cabs(delta)+csqrt(delta*delta+A.mat[ORDER-1][ORDER-2]*A.mat[ORDER-1][ORDER-2])));
    }
}
int tolcheck(matrix A){
    int t=0;
    double complex tol=1e-12;
    for (int i=1;i<ORDER;i++){
        for (int j=0;j<i;j++){
            if (cabs(A.mat[i][j])>cabs(tol)){
                t++;
                break;
            }
        }
    }
    if (t>0){
        return 1;
    } 
    else{return 0;}
}

QR QRDecomposition(matrix A){
    QR result;
    matrix  H;
    result.Q=Identity();
    result.R=A;    

    for (int k=0;k<ORDER-1;k++){
        double complex v[ORDER-k];
        double complex beta;
        for (int i=k;i<ORDER;i++){
            v[i-k]=result.R.mat[i][k];
        }
        double complex alpha;
        //printf("%lf %lf\n",creal(v[0]),creal(v[1]));  
        if (creal(v[0])==0){
        alpha=VectorNorm(v);
        }
        else{
        alpha=(result.R.mat[k][k]/cabs(result.R.mat[k][k]))*VectorNorm(v);
        }
        v[0]+=alpha;
        double complex vc=VectorNorm(v);
        if (vc!=0){
        for (int i=0;i<ORDER-k;i++){
            v[i]/=vc;
            }
        }
        double complex qprime[ORDER-k][ORDER-k];
        for(int i=0;i<ORDER-k;i++){
            for(int j=0;j<ORDER-k;j++){
                if (i==j){
                    qprime[j][i]=1-(2*conj(v[i])*v[j]);
                }
                else{
                    qprime[j][i]=-(2*conj(v[i])*v[j]);
                }
            }
        }
        

        for(int i=0;i<ORDER;i++){
            for (int j=0;j<ORDER;j++){
                if (i==j && i<k){H.mat[i][j]=1;}
                else if(i>=k&&j>=k){
                    H.mat[i][j]=qprime[i-k][j-k];
                }
                else{H.mat[i][j]=0;}
            }
        }
        result.R=MatMult(H,result.R);


        result.Q=MatMult(result.Q,trans(H));
    }


    return result;
}

double complex* QRAlgorithm(matrix A){
    double complex* eigenv = (double complex*)malloc(ORDER * sizeof(double complex));
    double complex shift;

    for (int  i=0; i< MAX_ITER; i++){
        if (tolcheck(A)==0){
            break;
        }
         shift = WilkinsonShift(A);
         matrix identity = Identity();
         A = MatSub(A, MatScalMult(identity, shift));
         QR qr = QRDecomposition(A);
         A = MatMult(qr.R,qr.Q);
        A=MatSub(MatMult(qr.R, qr.Q),MatScalMult(identity, -shift));       
    }
    
    int i;
    for (i=0;i<ORDER-1;i++){
        if (cabs(A.mat[i+1][i])>1e-5){
            double complex eig1,eig2,a=A.mat[i][i],b=A.mat[i][i+1],c=A.mat[i+1][i],d=A.mat[i+1][i+1];
            eigenv[i]=(a+d+csqrt((a+d)*(a+d)-4*(a*d-b*c)))/2;
            eigenv[i+1]=(a+d-csqrt((a+d)*(a+d)-4*(a*d-b*c)))/2;
            ++i;
            
        }
        else{
        eigenv[i]=A.mat[i][i];
        }
    }
    if (i==ORDER-1){
    eigenv[ORDER-1]=A.mat[ORDER-1][ORDER-1];}
    return eigenv;
}

double fx(double x) {
    return 2 * x + 32; // Derivative of f(x)
}

double f(double x) {
    return x * x + 32 * x - 273; // Original function
}

double* newton(void) {
    double tol = 1e-3; // Tolerance for convergence
    double* roots = (double*)malloc(2 * sizeof(double));
    double root = -100.0; // Initial guess for the first root
    roots[0] = root; // Initialize the first root

    // Find the first root
    while (fabs(f(root)) > tol) {
        root -= f(root) / fx(root); // Update using Newton-Raphson
    }
    roots[0] = root;

    // Find the second root
    root = 0.0; // Start from a different initial guess
    while (fabs(f(root)) > tol) {
        root -= f(root) / fx(root); // Update using Newton-Raphson
    }
    roots[1] = root;

    return roots;
}

