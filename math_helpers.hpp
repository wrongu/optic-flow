#pragma once

#include <cv.h>
#include <math.h>
#include <vector>
#include "functors.hpp"

using namespace cv;

typedef std::vector<double> vec_d;
typedef vec_d::size_type _int;
typedef vec_d::iterator _iter;


/* A la MatLab, get a vector of values */
vec_d linspace(double low, double high, int size);

vec_d map_func(vec_d src, DoubleFunc & fn);

// binary search for value of f(x), where f corresponds 1:1 to the given domain.
//	domain must be sorted.
double sample_func(vec_d domain, vec_d f, double x, _int i=1, _int j=0);

// O(1) search into linear domain
// 	domain must have even intervals
double linear_sample_func(vec_d domain, vec_d f, double x);

// squared euclidean distance between two vectors of the same size
double sq_dist(vec_d v1, vec_d v2);

void print_vec(double *vec, int n);

void print_vec(vec_d v);
