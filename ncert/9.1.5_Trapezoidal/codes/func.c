#include <stdlib.h>
#include <stdio.h>
#include <math.h>
// Define a structure to hold coordinates (x, y)
typedef struct coords{
	float x,y;
}coords;

// Function to compute the derivative dy/dx based on the given equation
float ffx(float y, float x){
	return -cos(3*x)/3+sin(3*x)/3;
}

// Function to compute the values of (x, y) using the Euler method
coords* fx(float yn,float x){
	float h=0.001;
	coords * f;
	f=(coords*)malloc(10000*sizeof(coords));
	// Initialize the first coordinates with the given initial values
	f[0].y=yn;
	f[0].x=x;
	for(int i=1;i<10000;i++){
		f[i].y=f[i-1].y+h*ffx(f[i-1].y,f[i-1].x); //y_(n+1)=y_n+h*ffx(y_n,x_n)
		f[i].x=f[i-1].x+h; //x_(n+1)=x_n+h
	}
	return f;
}
