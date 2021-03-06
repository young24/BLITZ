/*
* Copyright 2018 Wenbin Yang <bysin7@gmail.com>
* This file is part of BLITZ (Behavioral Learning In The Zebrafish),
* which is adapted from MindControl (Andrew Leifer et al <leifer@fas.harvard.edu>
* Leifer, A.M., Fang-Yen, C., Gershow, M., Alkema, M., and Samuel A. D.T.,
* 	"Optogenetic manipulation of neural activity with high spatial resolution in
*	freely moving Caenorhabditis elegans," Nature Methods, Submitted (2010).
*
* BLITZ is a free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the license, or
* (at your option) any later version.
*
* Filename: talk2camera.h
* Abstract: this file contains all function declarations
*			used in implementing Basler Pylon USB cameras
*
* Current Version: 2.0
* Author: Wenbin Yang <bysin7@gmail.com>
* Modified on: Apr. 28, 2018

* Replaced Version: 1.1
* Author: Wenbin Yang <bysin7@gmail.com>
* Created on: Jan. 1, 2018
*/


// Include user-defined libraries
#include "talk2camera.h"

using namespace std;
using namespace Pylon;


bool CameraData::initialize(int numCameras, int frameWidth, int frameHeight, int frameRate)
{
	const char* serialNums[MAX_CAMERAS] = { "21552672","22510229","22510230" };
	const int offSetX[MAX_CAMERAS] = { 737, 1153, 981 };
	const int offSetY[MAX_CAMERAS] = { 107, 49, 214 };
	Pylon::EPixelType pixelFormat = Pylon::EPixelType::PixelType_Mono8;

	Pylon::PylonInitialize();
	Pylon::CTlFactory&TlFactory = Pylon::CTlFactory::GetInstance();
	
	// Make sure cameras are attached in order
	for (int i = 0; i < numCameras; i++)
	{
		di[i].SetDeviceClass(Pylon::BaslerUsbDeviceClass);
		di[i].SetSerialNumber(serialNums[i]);
	}
	cameras.Initialize(numCameras);
	// Attach devices and set camera parameters
	for (int i = 0; i < numCameras; i++)
	{
		cameras[i].Attach(TlFactory.CreateDevice(di[i]));
		cameras[i].Open();
		cameras[i].Width = frameWidth;
		cameras[i].Height = frameHeight;
		cameras[i].OffsetX = offSetX[i];
		cameras[i].OffsetY = offSetY[i];
		cameras[i].AcquisitionFrameRateEnable.SetValue(true);
		cameras[i].AcquisitionFrameRate.SetValue(frameRate);
		// Print the model name of the camera.
		std::cout << "Using camera " << cameras[i].GetDeviceInfo().GetSerialNumber() << std::endl;
	}
	formatConverter.OutputPixelFormat = pixelFormat;
	cameras.StartGrabbing();
	return true;
}

/* Grab Pylon image from cameras */
bool CameraData::grabPylonImg()
{
	try{
		cameras.RetrieveResult(5000, ptrGrabResult, TimeoutHandling_ThrowException);
		if (!ptrGrabResult->GrabSucceeded())
		{
			cout << "Error: " << ptrGrabResult->GetErrorCode() << " " << ptrGrabResult->GetErrorDescription() << endl;
			return false;
		}
		// When the cameras in the array are created the camera context value
		// is set to the index of the camera in the array.
		// The camera context is a user settable value.
		// This value is attached to each grab result and can be used
		// to determine the camera that produced the grab result.
		cIdx = ptrGrabResult->GetCameraContext();
		formatConverter.Convert(pylonImg,ptrGrabResult);
	}
	catch (const GenericException &e)
	{
		// Error handling
		cerr << "An exception occurred." << endl
			<< e.GetDescription() << endl;
		return false;
	}
	return cameras.IsGrabbing();
}