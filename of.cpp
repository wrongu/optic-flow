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
	//	dst will be of type FLOW_TYPE(2)
	//		channel 0 is the row/vertical component of OF; channel 1 is column/horizontal
	//		*initialize dst to all 0s*
	//  img1 and img2 can be any type as long as they are the same.
	void continuation_method(const Mat & img1, const Mat & img2, Mat & dst, int k_max, double gamma, double alpha, double beta, int MAX_ITER, double eps){
		// check inputs
		CV_Assert(img1.rows == img2.rows && img1.cols == img2.cols);
		CV_Assert(k_max > 0);
		CV_Assert(gamma >= 0.0 && alpha >= 0.0 && beta >= 0.0);
		CV_Assert(MAX_ITER > 0);

		// set up pyramid
		Mat *scales1 = new Mat[k_max+1];
		scales1[k_max] = img1;
		Mat *scales2 = new Mat[k_max+1];
		scales2[k_max] - img2;
		double scale = 0.95;
		// This loop constructs multiple scales of the image such that
		//	scales[0] contains the lowest resolution and
		//	scales[k_max] contains the highest resolution
		for(int k=1; k<=k_max; ++k){
			// first frame
			Mat temp1;
			cv::resize(img1, temp1, Size(0,0), scale, scale);
			scales1[k_max-k] = temp1;
			// second frame
			Mat temp2;
			cv::resize(img2, temp2, Size(0,0), scale, scale);
			scales2[k_max-k] = temp2;
			scale = scale * 0.95;
		}

		// ===================
		// CONTINUATION METHOD
		// ===================
		Mat img2_offset, Iz, Ix, Iy, Ixz, Iyz, Ixx, Iyy, Ixy;
		Mat u, v, u_new, v_new;
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
			// u becomes u_new scaled up to finer resolution
			resize(u_new, u, im1.size());
			resize(v_new, v, im1.size());
			// du is used for the update: u(k+1) = u(k) + du(k)
			Mat du(im1.rows, im1.cols, FLOW_TYPE(1));
			Mat dv(im1.rows, im1.cols, FLOW_TYPE(1));
			flow_transform<float>(im2, im2_sampled, u, v);
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
			u_new = u + du;
			v_new = v + dv;
		}
		// TODO - final iteration with beta set to 0 to approximate the continuous limit
		delete[] scales1;
		delete[] scales2;
	}

	// dst(r,c) <- src(r + u(r,c), c + v(r,c))
	//	where vec_field(r,c) = (u, v)
	template <typename wtype>
	void flow_transform(const Mat & src, Mat & dst, const Mat & u, const Mat & v){
		Mat rows(u.rows, u.cols, u.type());
		Mat cols(u.rows, u.cols, u.type());
		// make row- and col-value matrices
		Mat col(u.rows, 1, u.type());
		for(int r=0; r<u.rows; ++r) col.at<wtype>(r,0) = r;
		Mat row(1, u.cols, u.type());
		for(int c=0; c<u.rows; ++c) row.at<wtype>(0,c) = c;
		// use a 'repmat' type tiling to build full matrices
		for(int r=0; r<u.rows; ++r)
			row.copyTo(rows(Rect(0,r,u.cols,1)));
		for(int c=0; c<u.cols; ++c)
			col.copyTo(cols(Rect(c,0,1,u.rows)));
		// mapping functions are indices + flow
		Mat mapx = cols + u;
		Mat mapy = rows + v;
		// remap dst from src and flow using opencv built-in function (linear interpolation and ignoring out of bounds values)
		remap(src, dst, mapx, mapy, INTER_LINEAR, BORDER_TRANSPARENT);
	}

	float psi_prime(float s_2, float eps_2){
		return 1.0f / (2.0f * sqrt(s_2 + eps_2));
	}

	void best_descriptor_match(const Mat & desc1, const Mat & desc2, Mat & u, Mat & v, const SparseSample & sp1, const SparseSample & sp2){
		std::cout << "entered best_descriptor_match" << std::endl;
		// create u and v at original sizes
		u.create(desc1.size(), FLOW_TYPE(1));
		v.create(desc1.size(), FLOW_TYPE(1));
		Size size1 = desc1.size();
		Size size2 = desc1.size();
		// reshape such that channels become rows, and wxh=N image flattened to Nx1
		Mat desc1_1D = desc1.reshape(1, desc1.rows*desc1.cols);
		Mat desc2_1D = desc2.reshape(1, desc2.rows*desc2.cols);
		// use built-in "brute force" matcher from opencv
		bool cross_check = true; // whether to ensure (i,j) match is transitive [ie (j,i) is best reverse match]
		BFMatcher::BFMatcher desc_matcher(NORM_L2, cross_check);
		vector<DMatch> matches;
		// get matches
		std::cout << "running match() on desc1_1D = " << desc1_1D.rows << "x" << desc1_1D.cols << " : " << _math::getImageType(desc1_1D.type()) << std::endl;
		std::cout << "\t and desc2_1D = " << desc2_1D.rows << "x" << desc2_1D.cols << " : " << _math::getImageType(desc2_1D.type()) << std::endl;
		desc_matcher.match(desc1_1D, desc2_1D, matches);
		std::cout << "building flow matrices from matches" << std::endl;
		for(int i=0; i<matches.size(); ++i){
			DMatch m = matches[i];
			int i1 = m.queryIdx; // 1D index from desc1
			int i2 = m.trainIdx; // 1D index from desc2
			// reshape 1D -> 2D indices
			int r1 = i1 / size1.width;
			int c1 = i1 % size1.width;
			r1 = sp1.sparse2Dense(r1);
			c1 = sp1.sparse2Dense(c1);
			int r2 = i2 / size2.width;
			int c2 = i2 % size2.width;
			r2 = sp2.sparse2Dense(r2);
			c2 = sp2.sparse2Dense(c2);
			// record match in u (horz flow) and v (vert flow)
			u.at<flow_t>(r1, c1) = (flow_t)(c2 - c1);
			v.at<flow_t>(r1, c1) = (flow_t)(r2 - r1);
		}
	}

	Mat overlay_field(const Mat & src, const Mat & u, const Mat & v){
		Mat result = src.clone();

		for(int r=0; r<src.rows; ++r){
			for(int c=0; c<src.cols; ++c){
				// rows    ~ vertical
				// columns ~ horizontal
				int v_component = u.at<flow_t>(r,c);
				int h_component = v.at<flow_t>(r,c);
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

	template <typename im_type>
				im_type SparseSample::at(Mat & sparse_src, int r, int c, im_type fillval){
		int sparse_r = dense2Sparse(r);
		int sparse_c = dense2Sparse(c);
		if(sparse2Dense(sparse_r) == r && sparse2Dense(sparse_c) == c)
			return sparse_src.at<im_type>(sparse_r, sparse_c);
		else
			return fillval;
	}
}


// TEST DRIVER
int of_exec(std::string file_in, std::string file_out, bool disp){
	cout << "of_exec entered" << endl;
	Mat img = imread(file_in, 1);

	if(!img.data){
		std::cerr << "Problem loading image: " << file_in << endl;
		return 1;
	}
	img.convertTo(img, FLOW_TYPE(3));

	// testing optic flow on img and rotated img:
	Mat rot_matrix = getRotationMatrix2D(Point2f(img.rows/2, img.cols/2), 2.0, 1.0);
	Mat img_rotated(img.rows, img.cols, img.type());
	warpAffine(img, img_rotated, rot_matrix, img_rotated.size());
	img_rotated.convertTo(img_rotated, FLOW_TYPE(3));

	cout << "image read. creating HOGFeature descriptors." << endl;
	of::SparseSample ss1(4, 0);
	of::SparseSample ss2(1, 0);
	// compute descriptors of first "frame" sparsely
	Mat HOG_descriptors = HOG_get_full_descriptors(img, 3, ss1);
	// compute descriptors of second "frame" at each pixel
	Mat HOG_descriptors2 = HOG_get_full_descriptors(img_rotated, 3, ss2);

	cout << "computing OF initialization. " << endl;

	Mat u, v;
	of::best_descriptor_match(HOG_descriptors, HOG_descriptors2, u, v, ss1, ss2);

	cout << "created descriptors.\nmaking overlay image" << endl;

	Mat overlay = of::overlay_field(img, u, v);

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
