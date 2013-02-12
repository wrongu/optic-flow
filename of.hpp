/*
 * of.hpp
 *
 *  Created on: Feb 6, 2013
 *      Author: Richard
 */

#ifndef OF_HPP_
#define OF_HPP_

#include "math_helpers.hpp"
#include <cv.h>
using namespace cv;

namespace of{

	class SparseSample;

	// this class encapsulates the translation from- and to- sparse coordinates
	//	with dense coordinates
	class SparseSample{
	public:
		SparseSample(int _spacing = 1, int _offset = 0) : spacing(_spacing), offset(_offset) {}
		int spacing;
		int offset;
		inline int dense2Sparse(int coord) const { return (coord-offset) / spacing; }
		inline int sparse2Dense(int coord) const { return coord * spacing + offset; }
	};

	const SparseSample NO_SPARSE;

	// function headers
	void best_descriptor_match(const Mat_<vec_d> & desc1, const Mat_<vec_d> & desc2, Mat & dst, const SparseSample & sp1 = NO_SPARSE, const SparseSample & sp2  = NO_SPARSE);
	void continuation_method(const Mat & img, Mat & dst, int k_max=4, double gamma=1.0, double alpha=1.0, double beta=1.0);
	Mat overlay_field(const Mat & src, const Mat & flow_field);
	Scalar HSV2BGR(Scalar hsv);

}

#endif /* OF_HPP_ */
