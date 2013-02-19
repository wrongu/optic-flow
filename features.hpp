//
//  features.hpp
//  
//
//  Created by Richard Lange on 2/4/13.
//  Copyright 2013 Dartmouth College. All rights reserved.
//
#pragma once

#include <stdlib.h>
#include <cv.h>
#include <highgui.h>
#include <iostream>
#include <vector>
#include "math_helpers.hpp"
#include "segmentation.hpp"
#include "of.hpp"

using namespace cv;
using std::vector;

// change these 3 lines for different sizes and types of features
#define FEAT_SIZE 15
#define LG_FEAT_SIZE 135
#define FEAT_TYPE float
#define CV_FEAT(n) CV_32FC(n)

typedef Vec<FEAT_TYPE, FEAT_SIZE> 	 feat_vec_t;
typedef Vec<FEAT_TYPE, LG_FEAT_SIZE> lg_feat_vec_t;

class CustomFeature;
class HOGFeature;

// function declarations
Mat HOG_get_full_descriptors(const Mat & img, int radius, const of::SparseSample & sampler = of::NO_SPARSE);

// The Feature class and its decendents provide quick access to pre-computed features.
class CustomFeature
{
public:
	CustomFeature(const Mat & img, int sz, const of::SparseSample & ss) : src(img), size(sz), sampler(ss){}
    virtual ~CustomFeature() {}
    // set to 0 makes it strictly virtual (only in subclasses)
    virtual feat_vec_t get_feature_at(int r, int c, bool index_is_sparse = false) const = 0;
    virtual feat_vec_t eval(int r, int c, int BORDER = BORDER_DEFAULT) = 0;
    virtual void sparseFilter2D(Mat & dst) = 0;
protected:
	const Mat src;
    int size;
    const of::SparseSample sampler;
	Mat precomputed;
};

class HOGFeature : public CustomFeature
{
public:
	HOGFeature(const Mat & img, int radius, const of::SparseSample & ss, int n = FEAT_SIZE);
	~HOGFeature();
	feat_vec_t get_feature_at(int r, int c, bool index_is_sparse = false) const;
	feat_vec_t eval(int r, int c, int BORDER = BORDER_DEFAULT);
    void sparseFilter2D(Mat & dst);
private:
    feat_vec_t eval_f(int r, int c, int BORDER = BORDER_DEFAULT);
    feat_vec_t eval_d(int r, int c, int BORDER = BORDER_DEFAULT);
    void initialize();
    int n_gradients;
    Mat *gradient_filters;
};

// driver function
int feat_exec(std::string file_in, std::string file_out, bool disp);
