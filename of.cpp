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
		// finite difference operators as convolution kernels, thanks to
		//	http://en.wikipedia.org/wiki/Finite_difference#Finite_difference_in_several_variables
//		Mat dx_mat = (Mat_<float>(1, 3) << 0.5, 0, 0.5); // 1 row x 3 col
//		Mat dy_mat = (Mat_<float>(3, 1) << 0.5, 0, 0.5); // 3 row x 1 col, but otherwise the same
//		Mat dxx_mat = (Mat_<float>(1, 3) << 1.0, -2.0, 1.0); // 2nd order center difference on x
//		Mat dyy_mat = (Mat_<float>(3, 1) << 1.0, -2.0, 1.0); // 2nd order center difference on y
//		Mat dxy_mat = (Mat_<float>(3, 3) << 1.0f, 0.0f,-1.0f,
//											0.0f, 0.0f, 0.0f,
//										   -1.0f, 0.0f, 1.0f) / 4.0f; // 2nd order cross xy
//
//		Ptr<BaseRowFilter> dx  = getLinearRowFilter   (CV_32F, CV_32F, dx_mat, -1, KERNEL_SMOOTH | KERNEL_SYMMETRICAL);
//		Ptr<BaseRowFilter> dy  = getLinearColumnFilter(CV_32F, CV_32F, dy_mat, -1, KERNEL_SMOOTH | KERNEL_SYMMETRICAL);
//		Ptr<BaseRowFilter> dxx = getLinearRowFilter   (CV_32F, CV_32F, dxx_mat, -1, getKernelType(dxx_mat, Point(-1, -1)));
//		Ptr<BaseRowFilter> dyy = getLinearColumnFilter(CV_32F, CV_32F, dyy_mat, -1, getKernelType(dyy_mat, Point(-1, -1)));
//		Ptr<BaseRowFilter> dxy = createLinearFilter   (CV_32F, CV_32F, dxy_mat);
		// TODO - reduce this function to just the iteration. Let all initialization be done in the calling functions
		//	to save time when processing live video
		Ptr<FilterEngine> dx  = createDerivFilter(CV_32F, CV_32F, 1, 0, 3);
		Ptr<FilterEngine> dy  = createDerivFilter(CV_32F, CV_32F, 0, 1, 3);
		Ptr<FilterEngine> dxx = createDerivFilter(CV_32F, CV_32F, 2, 0, 3);
		Ptr<FilterEngine> dyy = createDerivFilter(CV_32F, CV_32F, 0, 2, 3);
		Ptr<FilterEngine> dxy = createDerivFilter(CV_32F, CV_32F, 1, 1, 3);
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

	// TODO - move this back to math_helpers and figure out how to get it to still compile
	template <typename _Tp, int n>
	double sq_dist(Vec<_Tp, n> v1, Vec<_Tp, n> v2){
		double dist = 0.0;
		for(int i=0; i < n; ++i){
			double diff = v2[i] - v1[i];
			dist += diff * diff;
		}
		return dist;
	}

	void best_descriptor_match(const Mat & desc1, const Mat & desc2, Mat & dst, const SparseSample & sp1, const SparseSample & sp2){
		std::cout << "entered best_descriptor_match" << std::endl;
		for(int r=0; r<desc1.rows; ++r){
			std::cout << "desc. match row " << r << " of " << desc1.rows << std::endl;
			for(int c=0; c<desc1.cols; ++c){
				lg_feat_vec_t current_vec = desc1.at<lg_feat_vec_t>(r,c);
				int orig_r = sp1.sparse2Dense(r);
				int orig_c = sp1.sparse2Dense(c);
				// find best match in desc2
				int best_r = 0, best_c = 0;
				double best_dist = sq_dist<FEAT_TYPE, LG_FEAT_SIZE>(current_vec, desc2.at<lg_feat_vec_t>(0, 0));
				for(int i=0; i<desc2.rows; ++i){
					for(int j=0; j<desc2.cols; ++j){
						double next_dist = sq_dist<FEAT_TYPE, LG_FEAT_SIZE>(current_vec, desc2.at<lg_feat_vec_t>(i, j));
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
				// using Vec2f because dst was defined with type CV_32FC2
				dst.at<Vec2f>(orig_r, orig_c)[0] = (float) r_diff;
				dst.at<Vec2f>(orig_r, orig_c)[1] = (float) c_diff;
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


// TEST DRIVER
int of_exec(std::string file_in, std::string file_out, bool disp){
	cout << "feat_exec entered" << endl;
	Mat img = imread(file_in, 1);

	if(!img.data){
		std::cerr << "Problem loading image: " << file_in << endl;
		return 1;
	}
//	cout << "img read. has depth " << getImageType(img.depth()) << endl;
//	cout << "img read. has type " << getImageType(img.type()) << endl;
	img.convertTo(img, CV_FEAT(3));
//	cout << "img converted to floating pt" << endl;
//	cout << "img read. has depth " << getImageType(img.depth()) << endl;
//	cout << "img read. has type " << getImageType(img.type()) << endl;

	// testing optic flow on img and rotated img:
	Mat rot_matrix = getRotationMatrix2D(Point2f(img.rows/2, img.cols/2), 3.0, 1.0);
	Mat img_rotated(img.rows, img.cols, img.type());
	warpAffine(img, img_rotated, rot_matrix, img_rotated.size());
	img_rotated.convertTo(img_rotated, CV_FEAT(3));

	/*
	// Test image rotation (works)
	if(disp_output){
		namedWindow("original");
		namedWindow("rotated");
		imshow("original", img);
		imshow("rotated", img_rotated);
		waitKey(0);
		destroyWindow("original");
		destroyWindow("rotated");
	}
	 */

	cout << "image read. creating HOGFeature descriptors." << endl;
	of::SparseSample ss1(4, 0);
	of::SparseSample ss2(1, 0);
	// compute descriptors of first "frame" sparsely
	Mat HOG_descriptors = HOG_get_full_descriptors(img, 3, ss1);
	// compute descriptors of second "frame" at each pixel
	Mat HOG_descriptors2 = HOG_get_full_descriptors(img_rotated, 3, ss2);

	cout << "holy crap descriptors worked. computing OF initialization. " << endl;

	Mat flow_field(img.rows, img.cols, CV_32FC2);
	of::best_descriptor_match(HOG_descriptors, HOG_descriptors2, flow_field, ss1, ss2);

	cout << "created descriptors.\nmaking overlay image" << endl;

	Mat overlay = of::overlay_field(img, flow_field);

	if(disp){
		std::string title ="Optic Flow Visualization";
		namedWindow(title);
		imshow(title, overlay);
		waitKey(0);
		destroyWindow(title);
	}

	// SAVE FILE
	imwrite(file_out, overlay);

	cout << "--done--" << endl;

	return 0;
}
