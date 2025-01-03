#include <stdlib.h>
#include <stdio.h>
#include <math.h>
typedef struct coords{
	float x,y;
}coords;
float ffx(float y, float x){
	return exp(x)-y;
}
coords* fx(float yn,float x){
	float h=0.004;
	coords * f;
	f=(coords*)malloc(500*sizeof(coords));
	f[0].y=yn;
	f[0].x=x;
	for(int i=1;i<500;i++){
		f[i].y=f[i-1].y+h*ffx(f[i-1].y,f[i-1].x);
		f[i].x=f[i-1].x+h;
	}
	return f;
}
