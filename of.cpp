/*
 * of.cpp
 *
 *  Created on: Feb 6, 2013
 *      Author: Richard
 */

#include "of.hpp"

namespace of{

	// continuation_method(src, dst, ...)
	// 	this is an implementation of the algorithm outlined in "Large Displacement Optical Flow: Descriptor Matching in Variational Motion Estimation"
	//	dst should be of type CV_32FC2
	//		channel 0 is the row/vertical component of OF; channel 1 is column/horizontal
	//  img1 and img2 can be any type as long as they are the same.
	void continuation_method(const Mat & img1, const Mat & img2, Mat & dst, int k_max, double gamma, double alpha, double beta, int MAX_ITER, double eps){
		Mat *scales1 = new Mat[k_max+1];
		Mat *scales2 = new Mat[k_max+1];
		// This loop constructs multiple scales of the image such that
		//	scales[0] contains the lowest resolution and
		//	scales[k_max] contains the highest resolution
		for(int k=0; k<=k_max; k++){
			double scale = pow(0.95, (double)(k_max-k));
			// first frame
			Mat temp1;
			cv::resize(img1, temp1, Size(0,0), scale, scale);
			scales1[k] = temp1;
			// second frame
			Mat temp2;
			cv::resize(img2, temp2, Size(0,0), scale, scale);
			scales2[k] = temp2;
		}

		// ===================
		// CONTINUATION METHOD
		// ===================
		// loop over image scales, coarse to fine
		for(int k=0; k<=k_max; ++k){
			// 'delta' is our distance from the fixed point solution. break when it is within 'eps' of correct
			//	initialized to eps to ensure that the loop is entered
			double delta = eps;
			// fixed point iteration loop: loop until convergence (specified by 'eps') or max iterations
			for(int l=0; l < MAX_ITER && delta >= eps; ++l){

			}
		}
		// TODO - final iteration with beta set to 0 to approximate the continuous limit
		delete[] scales1;
		delete[] scales2;
	}

	void best_descriptor_match(const Mat_<vec_d> & desc1, const Mat_<vec_d> & desc2, Mat & dst, const SparseSample & sp1, const SparseSample & sp2){
		for(int r=0; r<desc1.rows; ++r){
			std::cout << "desc. match row " << r << " of " << desc1.rows << std::endl;
			for(int c=0; c<desc1.cols; ++c){
				vec_d current_vec = desc1(r,c);
				int orig_r = sp1.sparse2Dense(r);
				int orig_c = sp1.sparse2Dense(c);
				// find best match in desc2
				int best_r = 0, best_c = 0;
				double best_dist = sq_dist(current_vec, desc2(0, 0));
				for(int i=0; i<desc2.rows; ++i){
					for(int j=0; j<desc2.cols; ++j){
						double next_dist = sq_dist(current_vec, desc2(i, j));
						if(next_dist < best_dist){
							best_dist = next_dist;
							best_r = i;
							best_c = j;
						}
					}
				}
				// found best match in desc2 - get difference vector and assign it
				//	to destination matrix
				int r_diff = sp2.sparse2Dense(best_r) - orig_r;
				int c_diff = sp2.sparse2Dense(best_c) - orig_c;
				// using Vec2s because dst was defined with type CV_16UC2
				dst.at<Vec2s>(orig_r, orig_c)[0] = r_diff;
				dst.at<Vec2s>(orig_r, orig_c)[1] = c_diff;
				std::cout << orig_r << ", " << orig_c << " best match: " << sp2.sparse2Dense(best_r) << ", " << sp2.sparse2Dense(best_c) << std::endl;
			}
		}
		// TODO - reverse-check and discard any that don't match desc2 --> desc1 (as an optional check)
	}

	Mat overlay_field(const Mat & src, const Mat & flow_field){
		Mat result = src.clone();

		for(int r=0; r<src.rows; ++r){
			for(int c=0; c<src.cols; ++c){
				// rows    ~ vertical
				// columns ~ horizontal
				int v_component = flow_field.at<Vec2s>(r,c)[0];
				int h_component = flow_field.at<Vec2s>(r,c)[1];
				if(h_component != 0 && v_component != 0){
					double angle = atan2(v_component, h_component);
					Scalar hsv(angle * 57.29578, 1.0, 1.0); // 180 / pi approx. equals 57.29578
					Scalar bgr = HSV2BGR(hsv);
					line(result, Point(r,c), Point(r+v_component, c+h_component), bgr);
				}
			}
		}

		return result;
	}

	// thanks to http://en.wikipedia.org/wiki/HSL_and_HSV#Converting_to_RGB
	// 	for this algorithm
	Scalar HSV2BGR(Scalar hsv){
		double h = hsv[0];
		double s = hsv[1];
		double v = hsv[2];
		double chroma = s*v;
		double h2 = h/60;
		double x = chroma*(1 - abs((int)h2 % 2 - 1));
		double m = v - chroma;
		if(h2 < 1)
			return Scalar(chroma + m, x + m, m);
		else if(h2 < 2)
			return Scalar(x + m, chroma + m, m);
		else if(h2 < 3)
			return Scalar(m, chroma + m, x + m);
		else if(h2 < 4)
			return Scalar(m, x + m, chroma + m);
		else if(h2 < 5)
			return Scalar(x + m, m, chroma + m);
		else
			return Scalar(chroma + m, m, x + m);
	}
}
