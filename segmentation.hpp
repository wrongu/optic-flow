#pragma once

#include <cv.h>
#include <highgui.h>
#include <iostream>
#include <fstream>
#include "math_helpers.hpp"
#define NUM_TEX_FILTERS 17

using namespace cv;

// Function Headers
Mat apply_all_filters(const Mat & image, const Mat kernel_set[], const int n_filters);
Mat affinity(const Mat & image, Mat kernel_set[], const int n_filters);
double filter_max(Mat image, Point pt1, Point pt2);
Mat get_gaussian_derivative_filter(int m_width, double *sigma, double sample_radius, double theta, int n_deriv, int fine=4);
Mat get_center_difference_filter(int m_width, double sigma, double sample_radius, double scale=0.5, int fine=4);
Mat get_texture_channel(const Mat & img, int num_textures, int kmeans_attempts = 1);
Mat get_4_channels(const Mat & img, int num_textures = 32);
MatND get_1D_texture_vector(const Mat & img);
void disp_n_imgs(const Mat * imgs, int rows, int cols);
Mat G(const Mat& srouce, int bin_radius = 10);
Mat rotateImage(const Mat& source, double angle);
Mat get_blur_filter(int width);

// driver function
int seg_exec(std::string file_in, std::string file_out, bool disp);
