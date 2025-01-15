#include <stdlib.h>
#include <stdio.h>
#include <math.h>
//This code of gradient descent scans the entirety of the region to find global min and max
// Define a structure to hold coordinates (x, y)
typedef struct coords{
	double x,y;
}coords;
//Define struct to return global min and global max
typedef struct gradient{
	coords min,max;
}gradient;
// Function to compute the derivative df/dx based on the given equation
double f1x(double x){
	return 12*x*x*x-24*x*x+24*x-48;
}
//Function f(x)
double fx(double x){
	return 3*pow(x,4)-8*pow(x,3)+12*pow(x,2)-48*x+25;
}
//Gradient Descent
double gd(double cur,double up){
	double precision=0.0001,h=0.001;
	while((fabs(f1x(cur))>precision)&&(cur<up)){  //Ensuring the current iteration value is under upper bound and the loop breaks when slope is under precision value
		cur-=h*f1x(cur);  //gradient descent difference eqn
	}
	return cur;
}
//"Gradient Ascent"
double ga(double cur,double up){
        double precision=0.0001,h=0.001;
        while((fabs(f1x(cur))>precision)&&(cur<up)){  //Ensuring the current iteration value is under upper bound and the loop breaks when slope is under precision value
                cur+=h*f1x(cur);  //gradient ascent difference eqn
        }       
        return cur;
}
// Function to compute the values of global min and global max and selectively apply gradient descent and ascent
gradient g(double lower,double upper){
	double h=0.001,precision=0.0001;
	coords globalmax,globalmin;
	globalmax.y=-1e10,globalmin.y=1e10;//initialising
	while (upper>lower){ //terminating condition of scnanning the entire region
		if(f1x(lower)<0){
			if(fx(lower)>globalmax.y){
			globalmax.x=lower;
			globalmax.y=fx(lower);
			}
		double X=gd(lower,upper);
		if(fx(X)<globalmin.y){
			globalmin.x=X;
                        globalmin.y=fx(X);
		}
		lower=X;
		}
		else if(f1x(lower)>0){
                        if(fx(lower)<globalmin.y){
                        globalmin.x=lower;
                        globalmin.y=fx(lower);
                        }
                double XX=ga(lower,upper);
                if(fx(XX)>globalmax.y){
                        globalmax.x=XX;  
                        globalmax.y=fx(XX);
                }
                lower=XX;
                }
			lower+=0.1; //Not to get stuck
		

	}
	gradient val;
	val.max.y=globalmax.y;
	val.max.x=globalmax.x;
	val.min.y=globalmin.y;
	val.min.x=globalmin.x;
	return val; 
}
