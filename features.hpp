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

class CustomFeature;
class HOGFeature;

// function declarations
Mat_<vec_d> HOG_get_full_descriptors(const Mat & img, int radius, const of::SparseSample & sampler = of::NO_SPARSE);

// The Feature class and its decendents provide quick access to pre-computed features.
class CustomFeature
{
public:
	CustomFeature(const Mat & img, int sz, const of::SparseSample & ss) : src(img), size(sz), sampler(ss){}
    virtual ~CustomFeature() {}
    // set to 0 makes it strictly virtual (only in subclasses)
    const virtual vec_d & get_feature_at(int r, int c, bool index_is_sparse = false) const = 0;
    virtual vec_d eval(int r, int c, int BORDER = BORDER_DEFAULT) = 0;
    virtual void sparseFilter2D(Mat_<vec_d> & dst) = 0;
protected:
	const Mat src;
    int size;
    const of::SparseSample sampler;
	Mat_<vec_d> precomputed;
};

class HOGFeature : public CustomFeature
{
public:
	HOGFeature(const Mat & img, int radius, const of::SparseSample & ss, int n = 15);
	~HOGFeature();
	const vec_d & get_feature_at(int r, int c, bool index_is_sparse = false) const;
    vec_d eval(int r, int c, int BORDER = BORDER_DEFAULT);
    void sparseFilter2D(Mat_<vec_d> & dst);
private:
    void initialize();
    int n_gradients;
    Mat *gradient_filters;
};

// driver function
int feat_exec(std::string file_in, std::string file_out, bool disp);
