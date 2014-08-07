/*! 
* 	\file    thermocam.h
* 	\author  Gabriel Urbain <gurbain@mit.edu> - Visitor student at MIT SSL
* 	\date    July 2014
* 	\version 0.1
* 	\brief   Headers for ethernet thermographic camera
*
* 	License: The MIT License (MIT)
* 	Copyright (c) 2014, Massachussets Institute of Technology
*/

#ifndef THERMOCAM_HH
#define THERMOCAM_HH

// OpenCV libs
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/highgui/highgui.hpp"

// ORF libs
#include <PvSampleUtils.h>
#include <PvDevice.h>
#include <PvBuffer.h>
#include <PvStream.h>
#include <PvInterface.h>
#include <PvSystem.h>
#include <PvBufferWriter.h>
#include <PvPixelType.h>
#include <PvString.h>
#include <PvBufferWriter.h>
#include <PvBufferConverter.h>
#include <PvSystem.h>

// Common libs
#include <stdint.h>
#include <iomanip>
#include <limits>
#include <stdio.h>
#include <iostream>
#include <string>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fstream>
#include <signal.h>
#include <sys/stat.h>
#include <cstdio>
#include <vector>
#include <ctime>

// Project libs
#include "utils.h"

// Thermocam defines
#define BUFFER_COUNT (16)
#define DIRECTORY "img"

using namespace std;
using namespace cv;

class Thermocam {

	public:
		// Public functions
		Thermocam ();
		~Thermocam ();

		int init();
		void close();
		int capture(Mat& img, TimeStamp& ts);


	private:
		// Camera variables
		PvResult lResult;
		PvDeviceInfo *lDeviceInfo;
		PvSystem lSystem;
		PvUInt32 lInterfaceCount;
		PvDevice lDevice;
		PvGenParameterArray *lDeviceParams;
		PvGenInteger *lPayloadSize;
		PvGenCommand *lStart;
		PvGenCommand *lStop;
		PvGenParameterArray *lStreamParams;
		PvGenInteger *lCount;
		PvGenFloat *lFrameRate;
		PvGenFloat *lBandwidth;
		PvGenCommand *lResetTimestamp;
		PvStream lStream;
		PvBuffer *lBuffers;
		PvBuffer *lBuffer;
		PvImage *lImage;
		PvInt64 lWidth, lHeight;

		// Private functions
		int listCam();
		int connect();
		void startStream();
};

#endif
