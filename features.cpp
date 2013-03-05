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
	precomputed = Mat(sampler.dense2Sparse(src.rows)+1, sampler.dense2Sparse(src.cols)+1, CV_FEAT(n_gradients));
	if(n_gradients < 1) n_gradients = 1;
	gradient_filters = new Mat[n_gradients];
	double sigma[] = {5.0, 5.0};
	cout << "creating " << n_gradients << " filters" << endl;
	for (int i=0; i<n_gradients; ++i) {
		gradient_filters[i] = get_gaussian_derivative_filter(size*2+1, sigma, 13.0, 3.1415926 * i / (double)n_gradients, 1);
	}
	// compute all features on initialization. future access should be through get_feature_at
	sparseFilter2D(precomputed);
}

// Input: (r,c) from dense image
// Output: vec_d feature of nearest (floored) location in the sparse feature matrix
feat_vec_t HOGFeature::get_feature_at(int r, int c, bool index_is_sparse) const
{

//	cout << "precomputed's has size " << precomputed.rows << " x " << precomputed.cols << endl;
//	cout << "indices: (" << r << ", " << c << ") ";
	if(!index_is_sparse){
		r = sampler.dense2Sparse(r);
		c = sampler.dense2Sparse(c);
	}
//	cout << "sparse indices(" << r << ", " << c << ")\t";
	r = borderInterpolate(r, precomputed.rows, BORDER_REPLICATE);
	c = borderInterpolate(c, precomputed.cols, BORDER_REPLICATE);

	// cout << "precomputed's has size " << precomputed.rows << " x " << precomputed.cols << endl;
	// cout << "getting precomputed.at(" << r << ", " << c << ")" << endl;
	return precomputed.at<feat_vec_t>(r, c);
}

feat_vec_t HOGFeature::eval_f(int r, int c, int BORDER){
	feat_vec_t feature;
	// loop over each gaussian derivative filter
	for(int i=0; i<n_gradients; i++){
		// loop over local region in image for convolution
		for(int x=0; x<2*size+1; ++x){
			int sample_c = borderInterpolate(c-size+x, src.cols, BORDER);
			for(int y=0; y<2*size+1; ++y){
				int sample_r = borderInterpolate(r-size+y, src.rows, BORDER);
				// average values across image channels
				double sum_on_channels = 0.0;
				Vec3f value = src.at<Vec3f>(sample_r, sample_c);
				for(int ch=0; ch < src.channels(); ch++){
					sum_on_channels += (FEAT_TYPE)(value[ch] * gradient_filters[i].at<double>(y, x));
				}
				feature[i] += (FEAT_TYPE) sum_on_channels / src.channels();
			}
		}
	}
	return feature;
}

feat_vec_t HOGFeature::eval_d(int r, int c, int BORDER){
	feat_vec_t feature;
	// loop over each gaussian derivative filter
	for(int i=0; i<n_gradients; i++){
		// loop over local region in image for convolution
		for(int x=0; x<2*size+1; ++x){
			for(int y=0; y<2*size+1; ++y){
				int sample_r = borderInterpolate(r-size+y, src.rows, BORDER);
				int sample_c = borderInterpolate(c-size+x, src.cols, BORDER);
				// average values across image channels
				double sum_on_channels = 0.0;
				Vec3d value = src.at<Vec3d>(sample_r, sample_c);
				for(int ch=0; ch < src.channels(); ch++){
					sum_on_channels += (FEAT_TYPE)(value[ch] * gradient_filters[i].at<double>(y, x));
				}
				feature[i] += (FEAT_TYPE) sum_on_channels / src.channels();
			}
		}
	}
	return feature;
}

feat_vec_t HOGFeature::eval(int r, int c, int BORDER)
{
	// use float- or double-eval based on image type..
	//	I hope there is a better way to do this, but I don't see any since "types"
	//	like 'CV_32F' are macros that expand to integers..
	//	TODO - find that better way
	switch(src.depth()){
	case CV_8U:
	case CV_16U:
		src.convertTo(src, CV_FEAT(3));
		return eval(r, c, BORDER);
	case CV_32F:
		return eval_f(r, c, BORDER);
	case CV_64F:
		return eval_d(r, c, BORDER);
		return feat_vec_t();
	default:
		// print error and return 0-vector
		std::cerr << "eval problems" << endl;
		return feat_vec_t();
	}
}

void HOGFeature::sparseFilter2D(Mat & dst){
	cout << "entered sparseFilter2D" << endl;
	int sparseCols = sampler.dense2Sparse(src.cols) - 1,
			sparseRows = sampler.dense2Sparse(src.rows) - 1;

	// ensure proper size. this is O(1) if already the right size.
	dst.convertTo(dst, CV_FEAT(FEAT_SIZE));

	// sparse coordinates (r,c)
	// loop backwards for debugging (errors at high r,c)
	for(int r=sparseRows-1; r>=0; --r){
		cout << "row " << r+1 << " of " << sparseRows << endl;
		for(int c=sparseCols-1; c>=0; --c){
			// dense coordintates (y,x)
			int y = sampler.sparse2Dense(r);
			int x = sampler.sparse2Dense(c);
//			printf("sparseFilter2D : src has size %d x %d\taccessing (%d, %d)\n", src.rows, src.cols, y, x);
//			printf("sparseFilter2D : dst has size %d x %d\taccessing (%d, %d)\n", dst.rows, dst.cols, r, c);
			// TODO - use eval() instead of eval_d()
			dst.at<feat_vec_t>(r, c) = eval(y, x);
		}
	}
}

void vec_subcopy(lg_feat_vec_t & dst, const feat_vec_t & sub, int start){
	for(int i=0; i<sub.channels; ++i)
		dst[i + start] = sub[i];
}

Mat HOG_get_full_descriptors(const Mat & img, int radius, const of::SparseSample & sampler){
	int gradients = FEAT_SIZE;
	cout << "in HOG_get_full_descriptors - constructing features" << endl;
	HOGFeature feats(img, radius, sampler, gradients);
	int sparse_rows = img.rows / sampler.spacing;
	int sparse_cols = img.cols / sampler.spacing;
	Mat descriptors(sparse_rows, sparse_cols, CV_FEAT(LG_FEAT_SIZE));
	int step = 4 / sampler.spacing;
	step = step == 0 ? 1 : step;
	for(int r=0; r<sparse_rows; ++r){
		for(int c=0; c<sparse_cols; ++c){
			lg_feat_vec_t full_descriptor;
			feat_vec_t C = feats.get_feature_at(r, c, true);
			// append 8 surrounding neighbor features (named by directions NSEW)
			feat_vec_t N  = feats.get_feature_at(r-step, c, true);
			feat_vec_t NE = feats.get_feature_at(r-step, c+step, true);
			feat_vec_t E  = feats.get_feature_at(r, c+step, true);
			feat_vec_t SE = feats.get_feature_at(r+step, c+step, true);
			feat_vec_t S  = feats.get_feature_at(r+step, c, true);
			feat_vec_t SW = feats.get_feature_at(r+step, c-step, true);
			feat_vec_t W  = feats.get_feature_at(r, c-step, true);
			feat_vec_t NW = feats.get_feature_at(r-step, c-step, true);
			// copy subs to full descriptor
			vec_subcopy(full_descriptor, C,  0 * FEAT_SIZE);
			vec_subcopy(full_descriptor, N,  1 * FEAT_SIZE);
			vec_subcopy(full_descriptor, NE, 2 * FEAT_SIZE);
			vec_subcopy(full_descriptor, E,  3 * FEAT_SIZE);
			vec_subcopy(full_descriptor, SE, 4 * FEAT_SIZE);
			vec_subcopy(full_descriptor, S,  5 * FEAT_SIZE);
			vec_subcopy(full_descriptor, SW, 6 * FEAT_SIZE);
			vec_subcopy(full_descriptor, W,  7 * FEAT_SIZE);
			vec_subcopy(full_descriptor, NW, 8 * FEAT_SIZE);
			// SET RETURN VAL AT R, C
			descriptors.at<lg_feat_vec_t>(r,c) = full_descriptor;
		}
	}
	return descriptors;
}
