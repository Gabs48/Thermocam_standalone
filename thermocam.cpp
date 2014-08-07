/*! 
* 	\file    thermocam.cpp
* 	\author  Gabriel Urbain <gurbain@mit.edu> - Visitor student at MIT SSL
* 	\date    August 2014
* 	\version 0.1
* 	\brief   Sources for ethernet thermographic camera
*
* 	License: The MIT License (MIT)
* 	Copyright (c) 2014, Massachussets Institute of Technology
*/

#include "thermocam.h"

using namespace std;
using namespace cv;

Thermocam::Thermocam()
{
	lDeviceInfo = NULL;
	lWidth = 0;
	lHeight = 0;
	imgNum = 0;
	tslast = 0;
	timestamps = "timestampThermo.txt";
}

Thermocam::~Thermocam()
{}

int Thermocam::listCam()
{
	this->lSystem.SetDetectionTimeout( 100 );
	this->lResult = this->lSystem.Find();
	if( !lResult.IsOK() ) //search for ThermoCam
	{
		cout << "PvSystem::Find Error: " << lResult.GetCodeString().GetAscii();
		return -1;
	}
	this->lInterfaceCount = this->lSystem.GetInterfaceCount();

	for(PvUInt32 x = 0; x < this->lInterfaceCount; x++)
	{
		PvInterface * lInterface = lSystem.GetInterface(x);
		cout << "Ethernet Interface " << endl;
		cout << "IP Address: " << lInterface->GetIPAddress().GetAscii() << endl;
		cout << "Subnet Mask: " << lInterface->GetSubnetMask().GetAscii() << endl << endl;
		PvUInt32 lDeviceCount = lInterface->GetDeviceCount();
		for(PvUInt32 y = 0; y < lDeviceCount ; y++)
		{
			this->lDeviceInfo = lInterface->GetDeviceInfo(y);
			cout << "ThermoCam " << endl;
			cout << "IP Address: " << this->lDeviceInfo->GetIPAddress().GetAscii() << endl;
		}
	}

	return 0;
}

int Thermocam::connect()
{
	if(this->lDeviceInfo != NULL)
	{
		cout << "Connecting to " << this->lDeviceInfo->GetIPAddress().GetAscii() << endl;
		this->lResult = this->lDevice.Connect(this->lDeviceInfo); // connect to ThermoCam
		if ( !lResult.IsOK() ) {
			cout << "Unable to connect to " << this->lDeviceInfo->GetIPAddress().GetAscii() << endl;
			return -2;
		} else {
			cout << "Successfully connected to " << lDeviceInfo->GetIPAddress().GetAscii() << endl;
    			this->lDeviceParams = this->lDevice.GetGenParameters();
    			this->lPayloadSize = dynamic_cast<PvGenInteger *>(lDeviceParams->Get("PayloadSize"));
    			this->lStart = dynamic_cast<PvGenCommand *>(lDeviceParams->Get("AcquisitionStart"));
    			this->lStop = dynamic_cast<PvGenCommand *>(lDeviceParams->Get("AcquisitionStop"));
    			this->lDevice.NegotiatePacketSize();
			return 0;
		}
	} else {
		cout << "No device found" << endl;
		return -1;
	}
}


void Thermocam::startStream()
{
	cout << "Opening stream to ThermoCam" << endl;

	// Open the stream
	this->lStream.Open(lDeviceInfo->GetIPAddress());

	// Read device payload size
	PvInt64 lSize = 0;
	this->lPayloadSize->GetValue(lSize);

	// Set buffer size and count
	PvUInt32 lBufferCount = (this->lStream.GetQueuedBufferMaximum() < BUFFER_COUNT) ? 
	this->lStream.GetQueuedBufferMaximum() : 
	BUFFER_COUNT;
	this->lBuffers = new PvBuffer[lBufferCount];
	for (PvUInt32 i = 0; i < lBufferCount; i++) {
		lBuffers[i].Alloc(static_cast<PvUInt32>(lSize));
	}

	// Set device IP destination to the stream
	this->lDevice.SetStreamDestination(this->lStream.GetLocalIPAddress(), this->lStream.GetLocalPort()); 

	// Get stream parameters
	this->lStreamParams = this->lStream.GetParameters();
	this->lCount = dynamic_cast<PvGenInteger *>(this->lStreamParams->Get("ImagesCount"));
	this->lFrameRate = dynamic_cast<PvGenFloat *>(this->lStreamParams->Get("AcquisitionRate"));
	this->lBandwidth = dynamic_cast<PvGenFloat *>(this->lStreamParams->Get("Bandwidth"));
	for (PvUInt32 i = 0; i < lBufferCount; i++) {
		this->lStream.QueueBuffer(lBuffers + i);
	}
	this->lResetTimestamp = dynamic_cast<PvGenCommand *>(lDeviceParams->Get("GevTimestampControlReset"));
	this->lResetTimestamp->Execute();
	
	// Start the stream
	cout << "Sending StartAcquisition command to ThermoCam" << endl;
	this->lResult = this->lStart->Execute();

	// Get size parameters			
	this->lDeviceParams->GetIntegerValue("Width", this->lWidth);
	this->lDeviceParams->GetIntegerValue("Height", this->lHeight);

}			

void Thermocam::close()
{
	cout << "Sending AcquisitionStop command to ThermoCam" << endl;

	// Stop the stream
	this->lStop->Execute();

	// Empty and close the buffers
	this->lStream.AbortQueuedBuffers();
	while (this->lStream.GetQueuedBufferCount() > 0) {
		PvBuffer *lBuffer = NULL;
		PvResult lOperationResult;
	        lStream.RetrieveBuffer(&lBuffer, &lOperationResult);
	}
	cout << "Releasing buffers" << endl;
	delete []this->lBuffers; 

	// Close the stream
	cout << "Closing stream" << endl;
	this->lStream.Close(); 
	
	// Close the device
	cout << "Disconnecting ThermoCam" << endl;
	this->lDevice.Disconnect();
}

int Thermocam::init()
{	
	// Init
   	cout << "Connecting to and streaming data from ThermoCam" << endl;
	printf("pid %d\n", (int) getpid());  

	// Get connected devices list
	int retVal = this->listCam();
	if(retVal!=0)
		return -1;

	// Connect to the devices
	retVal = this->connect();
	if(retVal!=0)
		return -1;

	// Start the stream
	startStream();

	// Create a new timestamp file
	tsfile.open(timestamps.c_str());
	if (tsfile.is_open())
		tsfile<<endl<<endl<<"######################### NEW SESSION #######################"<<endl<<endl;

	return 0;	
}

int Thermocam::capture(Mat& img, TimeStamp& ts)
{

	// Buffer and images allocations
	this->lBuffer = NULL;
	this->lImage = NULL;
	PvResult lOperationResult;
	Mat rawlImage(cv::Size(lWidth, lHeight), CV_8U);

	// Start timestamp
	ts.start();

	// Get the buffer
	PvResult lResult = this->lStream.RetrieveBuffer(&this->lBuffer, &lOperationResult, 1000);

	// Check the buffer and fill image
	if (lResult.IsOK()) {
		if(lOperationResult.IsOK()) {
			if (this->lBuffer->GetPayloadType() == PvPayloadTypeImage) {
				this->lImage=this->lBuffer->GetImage();
				this->lBuffer->GetImage()->Alloc(lImage->GetWidth(), lImage->GetHeight(), PvPixelMono8);
			}
		}
		this->lImage->Attach(rawlImage.data, lImage->GetWidth(), lImage->GetHeight(), PvPixelMono8);
		this->lStream.QueueBuffer(lBuffer);
		img = rawlImage;
		return 0;
	} else {
		cout << "Timeout" << endl;
		return -1;
	}

	// Stop timestamp
	ts.stop();
}

int Thermocam::captureAndSave()
{
	// Create variables to save
	TimeStamp ts;
	Mat irImg;
	
	// Capture images
	this->capture(irImg, ts);
	
	//Save jpg images
	mkdir(DIRECTORY, 0777);
	stringstream ss;
	string name = "thermo";
	string type = ".png";
	ss<<DIRECTORY<<"/"<<name<<imgNum<<type;
	string filename = ss.str();
	ss.str("");
	
	try {
		imwrite(filename, irImg);
	} catch (int ex) {
		ERROR<<"Exception converting image to png format: "<<ex<<endl;
		return -1;
	}
	
	// Save time stamp
	if (!tsfile.is_open()) {
		tsfile.open(timestamps.c_str());
		if (!tsfile.is_open()) {
			ERROR<<"Impossible to open the file"<<endl;
			return -1;
		}
	}
	if (imgNum==0)
		INFO<<"Saving image files into folder "<<DIRECTORY<<endl;
	tsfile<<"IMAGENUM\t"<<imgNum<<"\tPROCTIME\t"<<ts.getProcTime()<<"\tMEANTIME\t"<<ts.getMeanTime()<<"\tDIFF\t"<<ts.getMeanTime()-tslast<<endl;

	imgNum++;
	tslast = ts.getMeanTime();
	
	return 0;


}
