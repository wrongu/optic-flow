/*
 * driver.hpp
 *
 *  Created on: Feb 5, 2013
 *      Author: Richard
 */

#include "segmentation.hpp"
#include "features.hpp"

int main(int argc, char** argv){
	char choice = 'f';
	if(argc > 1)
		choice = argv[1][0];

	switch(choice){
	case 's':
		return seg_exec(argc, argv);
	case 'f':
		return feat_exec(argc, argv);
	default:
		break;
	}
	return 0;
}
