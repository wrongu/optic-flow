/*
 * of.cpp
 *
 *  Created on: Feb 6, 2013
 *      Author: Richard
 */

#include "of.hpp"

/* TODO (having reviewed code organization in opencv/modules/gpu/optical_flow.cpp#BroxOpticalFlow)
 *  make (or use) image-pyramid type
 *  encapsulate algorithm in functor object
 ** gpu matrix operations
 */

namespace of{

	// continuation_method(src, dst, ...)
	// 	this is an implementation of the algorithm outlined in "Large Displacement Optical Flow: Descriptor Matching in Variational Motion Estimation"
	//	dst should be of type CV_32FC2
	//		channel 0 is the row/vertical component of OF; channel 1 is column/horizontal
	//		*initialize dst to all 0s*
	//  img1 and img2 can be any type as long as they are the same.
	void continuation_method(const Mat & img1, const Mat & img2, Mat & dst, int k_max, double gamma, double alpha, double beta, int MAX_ITER, double eps){
		Mat *scales1 = new Mat[k_max+1];
		Mat *scales2 = new Mat[k_max+1];
		Mat & w = dst; // the paper refers to the optic flow field as 'w', so here I alias it as such
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
		Mat img2_offset, Iz, Ix, Iy, Ixz, Iyz, Ixx, Iyy, Ixy;
		Mat psi1, psi2, psi3, psi4;
		// TODO - reduce this function to just the iteration. Let all initialization be done in the calling functions
		//	to save time when processing live video
		// TODO - gpu::createDerivFilter_GPU for parallel filtering
		Ptr<FilterEngine> dx  = createDerivFilter(CV_32F, CV_32F, 1, 0, 3);
		Ptr<FilterEngine> dy  = createDerivFilter(CV_32F, CV_32F, 0, 1, 3);
		Ptr<FilterEngine> dxx = createDerivFilter(CV_32F, CV_32F, 2, 0, 3);
		Ptr<FilterEngine> dyy = createDerivFilter(CV_32F, CV_32F, 0, 2, 3);
		Ptr<FilterEngine> dxy = createDerivFilter(CV_32F, CV_32F, 1, 1, 3);
		// img2_offest = img2(x + w)
		// loop over image scales, coarse to fine
		for(int k=0; k<=k_max; ++k){
			Mat im1 = scales1[k];
			Mat im2 = scales2[k];
			Mat im2_sampled(im1.rows, im1.cols, im1.type());
			Mat dw(im1.rows, im1.cols, w.type()); // dw is used for the update: w(k+1) = w(k) + dw(k)
			flow_transform<Vec3f, Vec2f>(im2, im2_sampled, w);
			// Fixed-point iteration 'constants' and pre-evaluated functions:
			Iz = im2_sampled - im1;
			(*dx).apply(im2_sampled, Ix);
			(*dy).apply(im2_sampled, Iy);
			(*dxx).apply(im2_sampled, Ixx);
			(*dyy).apply(im2_sampled, Iyy);
			(*dxy).apply(im2_sampled, Ixy);
			(*dx).apply(Iz, Ixz);
			(*dy).apply(Iz, Iyz);
			// 'delta_max' is our max change between iterations. break when it is within 'eps' of 0
			//	initialized to eps to ensure that the loop is entered
			double delta_max = eps;
			// fixed point iteration loop: loop until convergence (specified by 'eps') or max iterations
			for(int l=0; l < MAX_ITER && delta_max >= eps; ++l){

			}
		}
		// TODO - final iteration with beta set to 0 to approximate the continuous limit
		delete[] scales1;
		delete[] scales2;
	}

	// dst(r,c) <- src(r + u(r,c), c + v(r,c))
	//	where vec_field(r,c) = (u, v)
	template <typename im_vec_t, typename vec_t>
	void flow_transform(const Mat & src, Mat & dst, const Mat & vec_field){
		dst = src.clone();
		for(int r=0; r<src.rows; ++r){
			for(int c=0; c<src.cols; ++c){
				int sample_r = borderInterpolate(r + vec_field.at<vec_t>(r, c)[0], src.rows, BORDER_DEFAULT);
				int sample_c = borderInterpolate(c + vec_field.at<vec_t>(r, c)[1], src.cols, BORDER_DEFAULT);
				dst.at<im_vec_t>(r,c) = src.at<im_vec_t>(sample_r, sample_c);
			}
		}
	}

	float psi_prime(float s_2, float eps_2){
		return 1.0f / (2.0f * sqrt(s_2 + eps_2));
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
					Vec3d hsv(angle * 57.29578, 1.0, 1.0); // 180 / pi approx. equals 57.29578
					Vec3d bgr = HSV2BGR(hsv);
					line(result, Point(r,c), Point(r+v_component, c+h_component), Scalar(bgr[0], bgr[1], bgr[2]));
				}
			}
		}

		return result;
	}

	// thanks to http://en.wikipedia.org/wiki/HSL_and_HSV#Converting_to_RGB
	// 	for this algorithm
	Vec3d HSV2BGR(Vec3d hsv){
		double h = hsv[0];
		double s = hsv[1];
		double v = hsv[2];
		double chroma = s*v;
		double h2 = h/60;
		double x = chroma*(1 - abs((int)h2 % 2 - 1));
		double m = v - chroma;
		if(h2 < 1)
			return Vec3d(chroma + m, x + m, m);
		else if(h2 < 2)
			return Vec3d(x + m, chroma + m, m);
		else if(h2 < 3)
			return Vec3d(m, chroma + m, x + m);
		else if(h2 < 4)
			return Vec3d(m, x + m, chroma + m);
		else if(h2 < 5)
			return Vec3d(x + m, m, chroma + m);
		else
			return Vec3d(chroma + m, m, x + m);
	}

	void quiver(const Mat & img, const Mat & u, const Mat & v, Mat & dst){
		CV_Assert(img.rows == u.rows && img.cols == u.cols);
		CV_Assert(img.rows == v.rows && img.cols == v.cols);

		dst = img.clone();

		for(int r=0; r<img.rows; ++r){
			for(int c=0; c<img.cols; ++c){

			}
		}
	}


	void of_colorwheel(const Mat & u, const Mat & v, Mat & dst){
		CV_Assert(v.rows == u.rows && v.cols == u.cols);

		dst.create(u.rows, u.cols, CV_32FC3);

		for(int r=0; r<u.rows; ++r){
			for(int c=0; c<u.cols; ++c){
				double angle = atan2(v.at<double>(r,c), u.at<double>(r,c));
				double magnitude2 = v.at<double>(r,c)*v.at<double>(r,c) + u.at<double>(r,c)*u.at<double>(r,c);
				if(angle < 0) angle = angle + 6.28319; // add 2*pi for positive angle
				Vec3d hsv(angle * 57.29578, 1.0, magnitude2 / 12.0); // 180 / pi approx. equals 57.29578
				dst.at<Vec3d>(r,c) = HSV2BGR(hsv);
			}
		}
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
/*
	cout << "image read. creating HOGFeature descriptors." << endl;
	of::SparseSample ss1(4, 0);
	of::SparseSample ss2(1, 0);
	// compute descriptors of first "frame" sparsely
	Mat HOG_descriptors = HOG_get_full_descriptors(img, 3, ss1);
	// compute descriptors of second "frame" at each pixel
	Mat HOG_descriptors2 = HOG_get_full_descriptors(img_rotated, 3, ss2);

	cout << "computing OF initialization. " << endl;

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
*/

	float alpha = 80.0f;
	float gamma = 100.0f;
	float sigma = 0.8f;
	int inner = 3;
	int outer = 35;
	int solver = 10;
	gpu::BroxOpticalFlow optical_flow(alpha, gamma, sigma, inner, outer, solver);

	gpu::GpuMat frame0(img);
	gpu::GpuMat frame1(img_rotated);
	gpu::GpuMat u;
	gpu::GpuMat v;
	optical_flow(frame0, frame1, u, v);

	Mat colorwheel;
	Mat u_mat(u);
	Mat v_mat(v);
	of::of_colorwheel(u_mat, v_mat, colorwheel);
	imwrite(file_out, colorwheel);

	return 0;
}
