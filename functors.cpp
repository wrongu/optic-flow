/*
 * functors.cpp
 *
 *  Created on: Feb 5, 2013
 *      Author: Richard
 */

#include "functors.hpp"
#include <math.h>

// gaussians and derivatives
double gauss_exp(double x, double sigma2){
	return exp(-(x*x) / (2*sigma2));
}

double gauss1_exp(double x, double sigma2){
	return exp(-(x*x) / (2 * sigma2)) * -x / sigma2;
}

double gauss2_exp(double x, double sigma2){
	return exp(-(x*x) / (2 * sigma2)) * (x*x/sigma2 - 1);
}

double psi_deriv(double s_2, double eps_2){
	return 1.0 / (2.0 * sqrt(s_2 + eps_2));
}
