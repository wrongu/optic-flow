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
		return NONE;
	case INPUT:
		opts.file_in = str;
		return NONE;
	case ALGORITHM:
		switch(str[0]){
			case 'f':
				opts.algorithm = FEATURES;
				break;
			case 's':
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
			return OUTPUT;
		case 'f':
			return INPUT;
		case 'a':
			return ALGORITHM;
		case 'v':
			opts.display = true;
			break;
		case 'h':
			opts.display = false;
			break;
		case '-':
			switch(str[2]){
			case 'f':
				opts.algorithm = FEATURES;
				break;
			case 's':
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

	// run the algorithm
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
