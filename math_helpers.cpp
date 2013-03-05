/*
 * math_healpers.cpp
 *
 *  Created on: Feb 5, 2013
 *      Author: Richard Lange
 */

#include "math_helpers.hpp"

namespace _math{
	const double inf = 1000000.0;

	/* A la MatLab, get a vector of values */
	vec_d linspace(double low, double high, int size){
		if(size < 2) size = 2;
		vec_d ret(size);
		double space = (high - low) / (size-1);
		for(int i=0; i<size; i++){
			ret[i] = low + space * i;
		}
		return ret;
	}

	vec_d map_func(vec_d src, DoubleFunc & fn){
		vec_d dst(src.size());
		for(_int i=0; i<src.size(); ++i){
			//printf("fn(%f) = %f\n", src[i], fn( src[i] ));
			dst[i] = fn( src[i] );
		}
		return dst;
	}

	// binary search for value of f(x), where f corresponds 1:1 to the given domain.
	//	domain must be sorted.
	double sample_func(vec_d domain, vec_d f, double x, _int i, _int j){
		// base case
		if(j == 0 && i==1){
			j = domain.size()-1;
			i = 0;
		}else if(i == j || i > j)
			return f[i];

		// binary search
		_int q = (i + j) / 2;
		// too low - search higher
		if(domain[q] < x)
			return sample_func(domain, f, x, q+1, j);
		// too high - search lower
		else if(domain[q] > x)
			return sample_func(domain, f, x, i, q-1);
		// just right - return
		else
			return f[q];
	}

	// O(1) search into linear domain
	// 	domain must have even intervals
	double linear_sample_func(vec_d domain, vec_d f, double x){
		double start = domain[0];
		double end = domain[domain.size()-1];
		double frac = (x - start) / (end - start);
		vec_d::size_type index = domain.size() * frac;
		if(index < 0) index = 0;
		if(index > domain.size() - 1) index = domain.size() - 1;
		return f[index];
	}

	template <typename _Tp, int n>
	double sq_dist(Vec<_Tp, n> v1, Vec<_Tp, n> v2){
		double dist = 0.0;
		for(int i=0; i < n; ++i){
			double diff = v2[i] - v1[i];
			dist += diff * diff;
		}
		return dist;
	}

	template <typename _Tp, int n>
	void print_vec(Vec<_Tp, n> vec){
		std::cout << "[";
		for(int i = 0; i < n-1; i++){
			std::cout << vec[i] << "  ";
		}
		std::cout << vec[n-1] << "]\n";
	}

	void print_vec(double *vec, int n){
		std::cout << "[";
		for(int i = 0; i < n-1; i++){
			std::cout << vec[i] << "  ";
		}
		std::cout << vec[n-1] << "]\n";
	}

	void print_vec(vec_d v){
		std::cout << "[";
		for(_iter i=v.begin(); i+1 != v.end(); ++i){
			std::cout << *i << "  ";
		}
		std::cout << v[v.size()-1] << "]\n";
	}

	// thanks to stack overflow for int --> type name:
	// http://stackoverflow.com/questions/12335663/getting-enum-names-e-g-cv-32fc1-of-opencv-image-types
	std::string getImageType(int number)
	{
	    // find type
	    int imgTypeInt = number%8;
	    std::string imgTypeString;

	    switch (imgTypeInt)
	    {
	        case 0:
	            imgTypeString = "8U";
	            break;
	        case 1:
	            imgTypeString = "8S";
	            break;
	        case 2:
	            imgTypeString = "16U";
	            break;
	        case 3:
	            imgTypeString = "16S";
	            break;
	        case 4:
	            imgTypeString = "32S";
	            break;
	        case 5:
	            imgTypeString = "32F";
	            break;
	        case 6:
	            imgTypeString = "64F";
	            break;
	        default:
	            break;
	    }

	    // find channel
	    int channel = (number/8) + 1;

	    std::stringstream type;
	    type<<"CV_"<<imgTypeString<<"C"<<channel;

	    return type.str();
	}
}
