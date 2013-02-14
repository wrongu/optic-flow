/*
 * driver.cpp
 *
 *  Created on: Feb 5, 2013
 *      Author: Richard
 */

#include "segmentation.hpp"
#include "features.hpp"

enum ALGO {FEATURES, SEGMENTATION};
enum PARSE_CONTEXT {NONE, OUTPUT, INPUT, ALGORITHM};

struct options {
	bool display;
	ALGO algorithm;
	std::string file_in;
	std::string file_out;
};

PARSE_CONTEXT parse_opt(char* str, options & opts, PARSE_CONTEXT ctx = NONE){
	switch(ctx){
	case OUTPUT:
		opts.file_out = str;
		std::cout << "set output file to " << opts.file_out << std::endl;
		return NONE;
	case INPUT:
		opts.file_in = str;
		std::cout << "set input file to " << opts.file_out << std::endl;
		return NONE;
	case ALGORITHM:
		std::cout << "setting algorithm option" << std::endl;
		switch(str[0]){
			case 'f':
				std::cout << "set algo to features" << std::endl;
				opts.algorithm = FEATURES;
				break;
			case 's':
				std::cout << "set algo to segmentation" << std::endl;
				opts.algorithm = SEGMENTATION;
				break;
		}
		break;
	case NONE:
	default:
		break;
	}
	if(str[0] != '-'){
		// if no '-' is specified, it is a file name. in-file must be specified
		//	before the out file
		if(opts.file_in == "")
			opts.file_in = str;
		else
			opts.file_out = str;
	} else{
		// options:
		//	-o designate next string as output file
		//	-f designate next string as input file
		//	-v specify visible to true
		//	-h specify visible to false ('h'ide)
		//  -a designate next string as algorithm type
		//	--f specify features test
		//	--s specify segmentation test
		switch(str[1]){
		case 'o':
			std::cout << "set ctx to output" << std::endl;
			return OUTPUT;
		case 'f':
			std::cout << "set ctx to input" << std::endl;
			return INPUT;
		case 'a':
			std::cout << "set ctx to algorithm" << std::endl;
			return ALGORITHM;
		case 'v':
			std::cout << "set disp to true" << std::endl;
			opts.display = true;
			break;
		case 'h':
			std::cout << "set disp to false" << std::endl;
			opts.display = false;
			break;
		case '-':
			std::cout << "double option" << std::endl;
			switch(str[2]){
			case 'f':
				std::cout << "set algo to features" << std::endl;
				opts.algorithm = FEATURES;
				break;
			case 's':
				std::cout << "set algo to segmentation" << std::endl;
				opts.algorithm = SEGMENTATION;
				break;
			}
			break;
		default:
			break;
		}
	}
	return NONE;
}

int main(int argc, char** argv){
	// default options
	std::string default_file_in = "images/bakerlibrary.jpg";
	std::string default_file_out = "output/result.jpg";
	// set up 'options' struct, parse it
	options o = {true, FEATURES, "", ""};
	PARSE_CONTEXT next_ctx = NONE;
	for(int i=1; i < argc; ++i)
		next_ctx = parse_opt(argv[i], o, next_ctx);

	if(o.file_in == "")  o.file_in = default_file_in;
	if(o.file_out == "") o.file_out = default_file_out;

	// run the aglorithm
	switch(o.algorithm){
	case FEATURES:
		return feat_exec(o.file_in, o.file_out, o.display);
	case SEGMENTATION:
		return seg_exec(o.file_in, o.file_out, o.display);
	default:
		break;
	}

	std::cout << "no algorithm executed" << std::endl;

	return 0;
}
