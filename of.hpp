/*
 * of.hpp
 *
 *  Created on: Feb 6, 2013
 *      Author: Richard
 */

#ifndef OF_HPP_
#define OF_HPP_

#include <cv.h>
#include <opencv2/gpu/gpu.hpp>
#include "math_helpers.hpp"
using namespace cv;
using std::cout;
using std::cerr;
using std::endl;

namespace of{

	class SparseSample;
	class PsiDerivFilter;

	// this class encapsulates the translation from- and to- sparse coordinates
	//	with dense coordinates
	class SparseSample{
	public:
		SparseSample(int _spacing = 1, int _offset = 0) : spacing(_spacing), offset(_offset) {}
		inline int dense2Sparse(int coord) const { return (coord-offset) / spacing; }
		inline int sparse2Dense(int coord) const { return coord * spacing + offset; }
		int spacing;
		int offset;
	};

	class PsiDerivFilter : public BaseFilter
	{
	public:
		void PsiDerivFilter(){ eps = 0.0001; }
		void operator()(const uchar** src, uchar* dst, int dststep, int dstcount, int width, int cn);
	private:
		double eps;
	};

	const SparseSample NO_SPARSE;

	// function headers
	void best_descriptor_match(const Mat & desc1, const Mat & desc2, Mat & dst, const SparseSample & sp1 = NO_SPARSE, const SparseSample & sp2  = NO_SPARSE);
	void continuation_method(const Mat & img1, const Mat & img2, Mat & dst, int k_max=4, double gamma=1.0, double alpha=1.0, double beta=1.0, int MAX_ITER = 1000, double eps = 0.00001);
	Mat overlay_field(const Mat & src, const Mat & flow_field);
	Vec3d HSV2BGR(Vec3d hsv);
	template <typename im_vec_t>
		void flow_transform(const Mat & src, Mat & dst, const Mat & u, const Mat & v);
	void quiver(const Mat & img, const Mat & u, const Mat & v, Mat & dst);
	void of_colorwheel(const Mat & u, const Mat & v, Mat & dst);
}

// driver function
int of_exec(std::string file_in, std::string file_out, bool disp);

#include "features.hpp"
#include "segmentation.hpp"


#endif /* OF_HPP_ */
