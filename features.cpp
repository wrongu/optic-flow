//
//  features.cpp
//  
//
//  Created by Richard Lange on 2/4/13.
//  Copyright 2013 Dartmouth College. All rights reserved.
//

#include "features.hpp"

using namespace cv;
using std::cout;
using std::endl;

int fromSparse(int sparse_index, int offset, int spacing){ return offset + sparse_index * spacing; }
int toSparse(int index, int offset, int spacing){ return (index - offset) / spacing; }

HOGFeature::~HOGFeature(){
	delete[] gradient_filters;
}

HOGFeature::HOGFeature(const Mat & img, int radius, const of::SparseSample & ss, int n)
: CustomFeature(img, radius, ss), n_gradients(n)
{
	initialize();
}

void HOGFeature::initialize(){
	cout << "initialize() entered" << endl;
	precomputed = Mat_<vec_d>(sampler.dense2Sparse(src.rows), sampler.dense2Sparse(src.cols));
	if(n_gradients < 1) n_gradients = 1;
	gradient_filters = new Mat[n_gradients];
	double sigma[] = {5.0, 5.0};
	cout << "creating " << n_gradients << " filters" << endl;
	for (int i=0; i<n_gradients; ++i) {
		gradient_filters[i] = get_gaussian_derivative_filter(size*2+1, sigma, 13.0, 3.1415926 * i / (double)n_gradients, 1);
	}
	//    precomputed = new Mat_<vec_d>((src.rows - offset) / spacing, (src.cols - offset) / spacing, CV_64F);
	sparseFilter2D(precomputed);
}

// Input: (r,c) from dense image
// Output: vec_d feature of nearest (floored) location in the sparse feature matrix
vec_d HOGFeature::get_feature_at(int r, int c, bool index_is_sparse) const
{
	if(!index_is_sparse){
		r = sampler.dense2Sparse(r);
		c = sampler.dense2Sparse(c);
	}

	r = borderInterpolate(r, precomputed.rows, BORDER_REPLICATE);
	c = borderInterpolate(c, precomputed.cols, BORDER_REPLICATE);

	// cout << "precomputed's has size " << precomputed.rows << " x " << precomputed.cols << endl;
	// cout << "getting precomputed.at(" << r << ", " << c << ")" << endl;
	return precomputed(r, c);
}

vec_d HOGFeature::eval(int r, int c, int BORDER)
{
	vec_d feature;
	feature.reserve(n_gradients);
	// loop over each gaussian derivative filter
	for(int i=0; i<n_gradients; i++){
		// loop over local region in image for convolution
		for(int x=0; x<2*size+1; ++x){
			for(int y=0; y<2*size+1; ++y){
				int sample_r = borderInterpolate(r-size+y, src.rows, BORDER);
				int sample_c = borderInterpolate(c-size+x, src.cols, BORDER);
				// average values across image channels
				double sum_on_channels = 0.0;
				for(int ch=0; ch < src.channels(); ch++){
					double im_val = src.at<Vec3b>(sample_r, sample_c)[ch];
					sum_on_channels += im_val * gradient_filters[i].at<double>(y, x);
				}
				feature[i] += sum_on_channels / src.channels();
			}
		}
	}
	return feature;
}

void HOGFeature::sparseFilter2D(Mat_<vec_d> & dst){
	cout << "entered sparseFilter2D" << endl;
	int sparseCols = sampler.dense2Sparse(src.cols),
			sparseRows = sampler.dense2Sparse(src.rows);

	// sparse coordinates (r,c)
	for(int r=0; r<sparseRows; ++r){
		cout << "row " << r+1 << " of " << sparseRows << endl;
		for(int c=0; c<sparseCols; ++c){
			// dense coordintates (y,x)
			int y = sampler.sparse2Dense(r);
			int x = sampler.sparse2Dense(c);
			// printf("sparseFilter2D : src has size %d x %d\taccessing (%d, %d)\n", src.rows, src.cols, y, x);
			// printf("sparseFilter2D : dst has size %d x %d\taccessing (%d, %d)\n", dst.rows, dst.cols, r, c);
			// vec_d & val = dst(r,c);
			// cout << "dst(r,c) access check: " << &val << endl;
			dst(r, c) = eval(y, x);
		}
	}
}

Mat_<vec_d> HOG_get_full_descriptors(const Mat & img, int radius, const of::SparseSample & sampler){
	int gradients = 15;
	cout << "in HOG_get_full_descriptors - constructing features" << endl;
	HOGFeature feats(img, radius, sampler, gradients);
	int sparse_rows = img.rows / sampler.spacing;
	int sparse_cols = img.cols / sampler.spacing;
	Mat_<vec_d> descriptors(sparse_rows, sparse_cols);
	for(int r=0; r<sparse_rows; ++r){
		for(int c=0; c<sparse_cols; ++c){
			vec_d full_descriptor = feats.get_feature_at(r, c, true);
			// append 8 surrounding neighbor features (named by directions NSEW)
			vec_d N  = feats.get_feature_at(r-1, c, true);
			vec_d NE = feats.get_feature_at(r-1, c+1, true);
			vec_d E  = feats.get_feature_at(r, c+1, true);
			vec_d SE = feats.get_feature_at(r+1, c+1, true);
			vec_d S  = feats.get_feature_at(r+1, c, true);
			vec_d SW = feats.get_feature_at(r+1, c-1, true);
			vec_d W  = feats.get_feature_at(r, c-1, true);
			vec_d NW = feats.get_feature_at(r-1, c-1, true);
			full_descriptor.insert(full_descriptor.end(), N.begin(),  N.end());
			full_descriptor.insert(full_descriptor.end(), NE.begin(), NE.end());
			full_descriptor.insert(full_descriptor.end(), E.begin(),  E.end());
			full_descriptor.insert(full_descriptor.end(), SE.begin(), SE.end());
			full_descriptor.insert(full_descriptor.end(), S.begin(),  S.end());
			full_descriptor.insert(full_descriptor.end(), SW.begin(), SW.end());
			full_descriptor.insert(full_descriptor.end(), W.begin(),  W.end());
			full_descriptor.insert(full_descriptor.end(), NW.begin(), NW.end());

			// SET RETURN VAL AT R, C
			descriptors(r,c) = full_descriptor;
		}
	}
	return descriptors;
}

void parse_opt(char* opt_string, bool & vis_flag, std::string & out_file){
	if(opt_string[0] != '-') return;
	switch(opt_string[1]){
	case 'h':
		vis_flag = false;
		break;
	case 'o':
		out_file = &(opt_string[2]);
		break;
	}
}

// TEST DRIVER
int feat_exec(int argc, char** argv){
	cout << "feat_exec entered" << endl;

	bool disp_output = true;

	std::string f = "images/bakerlibrary.jpg";
	std::string out_file = "output/result.jpg";
	if(argc > 1)
		f = argv[1];
	for(int i=2; i < argc; ++i)
		parse_opt(argv[i], disp_output, out_file);

	Mat img = imread(f,1);

	if(!img.data){
		std::cerr << "Problem loading image: " << f << endl;
		return 1;
	}

	// testing optic flow on img and rotated img:
	Mat rot_matrix = getRotationMatrix2D(Point2f(img.rows/2, img.cols/2), 3.0, 1.0);
	Mat img_rotated(img.rows, img.cols, img.type());
	warpAffine(img, img_rotated, rot_matrix, img_rotated.size());

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
	Mat_<vec_d> HOG_descriptors = HOG_get_full_descriptors(img, 3, ss1);
	// compute descriptors of second "frame" at each pixel
	Mat_<vec_d> HOG_descriptors2 = HOG_get_full_descriptors(img_rotated, 3, ss2);

	cout << "holy crap descriptors worked. computing OF initialization. " << endl;

	Mat flow_field(img.rows, img.cols, CV_16UC2);
	of::best_descriptor_match(HOG_descriptors, HOG_descriptors2, flow_field, ss1, ss2);

	cout << "created descriptors.\nmaking overlay image" << endl;

	Mat overlay = of::overlay_field(img, flow_field);

	if(disp_output){
		std::string title ="Optic Flow Visualization";
		namedWindow(title);
		imshow(title, overlay);
		waitKey(0);
		destroyWindow(title);
	}

	// SAVE FILE
	imwrite(out_file, overlay);

	cout << "--done--" << endl;

	return 0;
}
