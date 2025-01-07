#include <stdio.h>
#include <math.h>

double integrated(double x1,double x2){
	int N=300000; //Number of iterations
	double h=(x2-x1)/N, A=0.0,x=1.0;
	for(int i=0;i<N;i++){
		A+=(( sqrt(x+h) + sqrt(x) )/2)*h; //trapezoidal rule
		x+=h; //increment of x
	}
	return A;
}
