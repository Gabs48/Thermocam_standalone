/*! 
* 	\file    main.cpp
* 	\author  Gabriel Urbain <gurbain@mit.edu> - Visitor student at MIT SSL
* 	\date    August 2014
* 	\version 0.1
* 	\brief   Main program for optical range finder and stereo cameras acquisition
*
* 	License: The MIT License (MIT)
* 	Copyright (c) 2014, Massachussets Institute of Technology
*/

// Project libs
#include "thermocam.h"
#include "utils.h"

using namespace std;
using namespace cv;

/* 
 * Thermocam utilization example:
 * - Create
 * - Initialize
 * - In a loop: capture
 * - Close
 */

// Create variables
Thermocam thermo;

// Create a CTRL-C handler
void my_handler(int s){
	thermo.close();
	exit(1);
}

// Main program: in case there is no CTRL-C handler, ORF should be declared inside 
int main(int argc, char **argv) {

	// Display soft infos
	init();

	// For catching a CTRL-C
	signal(SIGINT,my_handler);
	
	// Initialize camera
	int retVal = thermo.init();
	if (retVal!=0)
		return -1;

	while(true) {

		// Capture and save image
		thermo.captureAndSave();
		
		usleep(100000);

	}
	
	// Close the camera
	thermo.close();	

	return 0;
}
