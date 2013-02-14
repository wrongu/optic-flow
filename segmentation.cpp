#include "segmentation.hpp"

using std::cout;
using std::endl;

Mat apply_all_filters(const Mat & image, const Mat kernel_set[], const int n_filters){
	// construct N-channel matrix from convolution with kernels
	Mat *channels = new Mat[n_filters];
	for(int i=0; i<n_filters; i++){
		cout << "\tapplying kernel" << i << endl;
		channels[i] = Mat(image.cols, image.rows, CV_64F);
		filter2D(image, channels[i], -1, kernel_set[i]);
	}

	//disp_n_imgs(channels, 4, 4);

/*
	// DISPLAY
	Mat kernel_resized(100,100,CV_64F);
	namedWindow("Filtered Image");
	namedWindow("Filter");
	for(int i=0; i<n_filters; i++){
		cv::resize(0.5*(kernel_set[i] + 1), kernel_resized, cv::Size(100,100));
		imshow("Filtered Image", channels[i]);
		imshow("Filter", kernel_resized);
		waitKey(0);
	}
	kernel_resized.release();
*/
	Mat ret(image.cols, image.rows, CV_64F);
	merge(channels, n_filters, ret);
	delete[] channels;
	return ret;
}

// get affinity matrix from given image and set of filters
Mat affinity(const MatND & img4channel){
	int N = img4channel.rows * img4channel.cols;
	Mat aff = Mat::eye(N,N,CV_64F);
	
	// loop over all pairs of pixels
	for(int i=0; i<N; ++i){
		for(int j = i+1; j < N; ++j){
			
		}
	}
	
	return aff;
}

// maximum sPd on line
double filter_max(Mat image, Point pt1, Point pt2){
	return 0.0;
}

/* FILTERS
 *  As defined in eqns (9).. in "Arbelaez" 2010
 *  On this computer: /Users/Richard/Documents/_CLASSES/thesis/code/optic flow/brox10/Berkeley Segmentation/documentation
 */

// implementation based on http://www.eecs.berkeley.edu/Research/Projects/CS/vision/bsds/code/Filters/oeFilter.m
//	Creates an nth-derivative gaussian edge filter
// INPUTS:
// 		m_width:   output matrix will be m_width x m_width. width should be negative.
// 		sigma:     an array with 2 values: sigmax and sigmay
//		sample_radius: x,y data sampled from [-sample_radius, sample_radius]
//		theta:   radians measure of rotation in [0, pi)
//		1: 0 for gaussian, 1 for 1st deriv y, 2 for 2nd deriv y
Mat get_gaussian_derivative_filter(int m_width, double *sigma, double sample_radius, double theta, int n_deriv, int fine){
	// TODO - use finer resolution for averaging?
	Mat ret = Mat::zeros(m_width, m_width, CV_64F);
	
	if(fine < 1) fine = 1;
	int n = fine*m_width;
	double root2 = 1.415;
	// more than sqrt(2) wider so it can handle rotation
	vec_d domain_fine = linspace(-sample_radius*root2, sample_radius*root2, n);
	vec_d domain = linspace(-sample_radius, sample_radius, m_width);
	
	// square sigma values here to reduce multiplications
	double sigma2x = sigma[0] * sigma[0];
	double sigma2y = sigma[1] * sigma[1];
	
	// create x- and y-curves for un-rotated filter (to be sampled later according to domain_fine)
	//	GaussDerivFunc is a functor object [i.e. it has an overloaded operator() method so it can do function evaluation]
	// x is always 0th derivative. y's varies, (can be rotated by 90 for x)
	GaussDerivFunc gauss_x (0, sigma2x);
	GaussDerivFunc gauss_y (n_deriv, sigma2y);
	
	// get 1D curves for each x and y (convolved in loop below)
	vec_d x_bell = map_func(domain_fine, gauss_x);
	vec_d y_bell = map_func(domain_fine, gauss_y);
	
	// output x and y curves for testing
	//print_vec(x_bell);
	//print_vec(y_bell);
	
	// sine and cosine used for rotating sampled points
	double c = cos(theta);
	double s = sin(theta);
	
	// convolve x and y curves to make 2D gaussian surface
	for(int xi=0; xi<m_width; ++xi){
		for(int yi=0; yi<m_width; ++yi){
			double x = domain[xi];
			double y = domain[yi];
			
			double rotx = x*c - y*s;
			double roty = x*s + y*c;
			
			// cout << "[%f, %f, %f]\n", domain_fine[0], rotx, *(domain_fine.end()-1));
			double fx = linear_sample_func(domain_fine, x_bell, rotx);
			double fy = linear_sample_func(domain_fine, y_bell, roty);
			
			// Image is as a matrix, so y is rows and x is columns
			ret.at<double>(yi,xi) = fx * fy;
		}
	}
	
	return ret;
}

// center-difference isn't convolution - it's a 2nd derivative gaussian revolved around (0,0)
Mat get_center_difference_filter(int m_width, double sigma, double sample_radius, double scale, int fine){
	// TODO - use finer resolution for averaging?
	Mat ret = Mat::zeros(m_width, m_width, CV_64F);
	
	double n = fine * m_width;
	double root2 = 1.415;
	vec_d domain_fine = linspace(-sample_radius*root2, sample_radius*root2, n);
	vec_d domain = linspace(-sample_radius, sample_radius, m_width);
	
	// square sigma values here to reduce multiplications
	double sigma2 = sigma * sigma;
	
	// here, just convolving 2nd derivative with itself.
	GaussDerivFunc gauss (2, sigma2);
	
	// get 1D curve
	vec_d bell = map_func(domain_fine, gauss);
	
	// revolve curve
	for(int xi=0; xi<m_width; ++xi){
		for(int yi=0; yi<m_width; ++yi){
			double x = domain[xi];
			double y = domain[yi];
			
			// revolved, so 'x' is really just dist-to-zero
			double r = sqrt(x*x + y*y);
			double fx = linear_sample_func(domain_fine, bell, r);
			
			// y is rows and x is columns in the matrix
			ret.at<double>(yi,xi) = fx*scale;
		}
	}
	
	return ret;
}

Mat get_texture_channel(const Mat & img, int num_textures, int kmeans_attempts){
	Mat img_gray(img.rows, img.cols, CV_32F);
	cvtColor(img,img_gray,CV_RGB2GRAY);
	
	Mat text_vec = get_1D_texture_vector(img_gray);
	Mat labels;
	// sample code thanks to http://stackoverflow.com/questions/10240912/input-matrix-to-opencv-kmeans-clustering
	cout << "running kmeans" << endl;
	kmeans(text_vec, num_textures, labels, TermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS, 0.0001, 10000), 
			kmeans_attempts, KMEANS_PP_CENTERS);
	cout << "kmeans done" << endl;
	
	labels.convertTo(labels, CV_8U);
	Mat ret = labels.reshape(1, img.rows);

	return ret;
}

/* Given RGB image, returns image with 4 channels, as described in the Alberaez paper.
 * Channel 0: L
 * Channel 1: a
 * Channel 2: b
 * Channel 3: texture
 *
 * Where 'texture' is a channel of cluster labels, where clusters come from kmeans run
 * 	on the 17-channel image that is the result of convolution with 17 gaussian derivative
 *	filters.
 */
Mat get_4_channels(const Mat & img, int num_textures){
	Mat imgLab(img.rows, img.cols, CV_64FC3);
	// convert to L*a*b space for first 3 channels
	cout << "getting Lab colors\n" << endl;
	cvtColor(img, imgLab, CV_RGB2Lab);
	// get last channel: texture
	cout << "getting texture channel\n" << endl;
	Mat tex = get_texture_channel(img, num_textures);
	
	cout << "building 4-channel matrix\n" << endl;
	Mat * splits = new Mat[4];
	split(imgLab, splits);
	splits[3] = tex;
	
	Mat ret(img.cols, img.rows, CV_64F);
	merge(splits, 4, ret);
	free(splits);
	return ret;
}

Mat get_1D_texture_vector(const Mat & img){
	
	// create 17-channel return matrix where each row corresponds to a pixel from the image
	//	kmeans likes 32F, not 64.
	
	double theta [] = {0, 0.4488, 0.8976, 1.3464, 1.7952, 2.2440, 2.6928, 3.1416};
	int w = 13;
	double sigma [] = {5.0, 5.0};
	double sample_radius = 15.0;
	Mat kernels [] = {	// a single center-difference filter
						get_center_difference_filter(w, sigma[0], sample_radius),
					    // 8 1st deriv
						get_gaussian_derivative_filter(w, sigma, sample_radius, theta[0], 1),
					   	get_gaussian_derivative_filter(w, sigma, sample_radius, theta[1], 1),
					   	get_gaussian_derivative_filter(w, sigma, sample_radius, theta[2], 1),
					   	get_gaussian_derivative_filter(w, sigma, sample_radius, theta[3], 1),
					   	get_gaussian_derivative_filter(w, sigma, sample_radius, theta[4], 1),
					   	get_gaussian_derivative_filter(w, sigma, sample_radius, theta[5], 1),
					   	get_gaussian_derivative_filter(w, sigma, sample_radius, theta[6], 1),
						get_gaussian_derivative_filter(w, sigma, sample_radius, theta[7], 1),
						// 8 2nd deriv
					 	get_gaussian_derivative_filter(w, sigma, sample_radius, theta[0], 2),
					   	get_gaussian_derivative_filter(w, sigma, sample_radius, theta[1], 2),
					   	get_gaussian_derivative_filter(w, sigma, sample_radius, theta[2], 2),
					   	get_gaussian_derivative_filter(w, sigma, sample_radius, theta[3], 2),
					   	get_gaussian_derivative_filter(w, sigma, sample_radius, theta[4], 2),
					   	get_gaussian_derivative_filter(w, sigma, sample_radius, theta[5], 2),
					   	get_gaussian_derivative_filter(w, sigma, sample_radius, theta[6], 2),
						get_gaussian_derivative_filter(w, sigma, sample_radius, theta[7], 2)};
						
	Mat img17d = apply_all_filters(img, kernels, NUM_TEX_FILTERS);
	
	//cout << "img17d is %d x %d with ch = %d\n", img17d.rows, img17d.cols, img17d.channels());
	
	// convert the 17 2D matrices to a single Nx17 matrix
	cout << "creating Nx17 vector for kmeans\n" << endl;
	
	Mat ret = img17d.reshape(1, img17d.rows*img17d.cols);
	ret.convertTo(ret, CV_32F);
	
	//cout << "ret is %d x %d with ch = %d\n", ret.rows, ret.cols, ret.channels());
	
	// cout << "type of ret is %d. CV_32F is %d\n", ret.type() , CV_32F);
	
	cout << "vector created.\n" << endl;
	return ret;
}

Mat disp_n_imgs(const Mat * imgs, int rows, int cols, double * brightness, bool window){
	
	int w = imgs[0].cols;
	int h = imgs[0].rows;
	
	Mat block_img(h*rows, w*cols, CV_64F, 0.5);
	Mat tmp_copy(h, w, CV_64F);
	
	int i = 0;
	for(int r = 0; r < rows; ++r){
		for(int c=0; c < cols; ++c){
			imgs[i].convertTo(tmp_copy, CV_64F, brightness[i]);
			tmp_copy.copyTo(block_img(Rect(c * w, r * h, w, h)));
			++i;
		}
	}
	
	//cout << "depth is %d\t 32F is %d\n", block_img.depth(), CV_32F);
	char * name;
	const char * fmt = std::string("%d channel image").c_str();
	sprintf(name, fmt, rows*cols);
	
	if(window){
		namedWindow(name);
		imshow(name, block_img);
		waitKey(0);
		destroyWindow(name);
	}

	return block_img;
}

// thanks to http://stackoverflow.com/questions/2289690/opencv-how-to-rotate-iplimage
Mat rotateImage(const Mat& source, double angle)
{
    Point2f src_center(source.cols/2.0F, source.rows/2.0F);
    Mat rot_mat = getRotationMatrix2D(src_center, angle, 1.0);
    Mat dst;
    warpAffine(source, dst, rot_mat, source.size());
    return dst;
}

Mat get_blur_filter(int width){
	Mat ret(width, width, CV_64F);
	ret = 1.0 / (double)(width * width);; // overloaded assignment operator sets all values to given scalar
	return ret;
}

int seg_exec(std::string file_in, std::string file_out, bool disp){
	cout << "reading image" << endl;
	Mat img = imread(file_in, 1);
	
	Mat four_channel = get_4_channels(img);
	
	Mat * chs = new Mat[4];
	split(four_channel, chs);
	double brightness [] = {1/255.0, 1/255.0, 1/255.0, 1/40.0};
	Mat result = disp_n_imgs(chs, 2, 2, brightness, disp);
	
	imwrite(file_out, result);

	// TODO - oriented histograms?
	delete[] chs;
	cout << "DONE" << endl;
	return 0;
}
