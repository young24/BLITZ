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
* Filename: main.cpp
* Abstract: this file contains all functions used in constructing final
*			behavioral learning experiment in zebrafish
*
* Current Version: 2.0
* Author: Wenbin Yang <bysin7@gmail.com>
* Modified on: Apr. 28, 2018

* Replaced Version: 1.1
* Author: Wenbin Yang <bysin7@gmail.com>
* Created on: Jan. 1, 2018
*/
// Include 3rd party libraries
#include "3rdPartyLibs/Utilities/Timer.h"

// Include user-defined libraries
#include "MyLibs/experiment.h"
#include "MyLibs/talk2screen.h"
#include "MyLibs/talk2camera.h"


// Include standard libraries
#include <iostream>

using namespace std;
using namespace cv;

int main()
{

	string CS_Pattern = "redBlackCheckerboard";
	ExperimentData exp(CS_Pattern);

	if (!exp.initialize())
	{
		cout << "Experiment Initialization Failed." << endl;
		exit(0);
	}
	else {
		cout << "Experiment initialized." << endl;
	}

	exp.runOLexp();



	
	


	/*Test screen function*/
	/*		Area1			  Area2				  Area3 
	  (0.233f, 0.300f)  (0.800f, -0.850f)  (-0.740f, -0.850f)
	  width, height: all (0.28f, 1.40f)  
	




	const char imgName[] = "Images/redCheckerboard.jpg";
	float allAreaPos[3][2] = { {0.233f, 0.300f}, {0.800f, -0.850f}, {-0.740f, -0.850f} };
	
	
	Timer expTimer;
	expTimer.start();
	ScreenData screen;
	screen.initGLFWenvironment();
	screen.loadTextureIntoBuffers(imgName);
	float* areaPos = allAreaPos[0];
	AreaData area1(areaPos,2);
	const int delimYarr[] = { 900,900,1000,1000 };
	area1.initialize(delimYarr);
	screen.allAreas.push_back(area1);



	while (1)
	{
		int timeInSec = expTimer.getElapsedTimeInSec();
		cout << "Time (s) : " << timeInSec << endl;
		if (timeInSec % 10 == 0)
			screen.allAreas[0].allPatches[0].pIdx = !screen.allAreas[0].allPatches[0].pIdx;
		screen.allAreas[0].allPatches[0].updatePattern();
		screen.renderTexture();
	}
	*/




	/* Test OL Procedure 
	ExperimentData exp;
	
	const char imgName[] = "Images/redCheckerBoard.jpg";
	try {
		exp.initialize(imgName);
		exp.prepareBgImg();
		exp.runOLexp();
	}
	catch (const GenericException &e)
	{
		// Error handling
		cerr << "An exception occurred." << endl
			<< e.GetDescription() << endl;
	}
	*/


	/*
	
	int testVar = 10;
	vector<int> headVec(4, 0);
	headVec[1] = 1;
	headVec[2] = 2;
	headVec[3] = 3;
	fs << "Frame" << "[";
	string vName; // variable name
	for (int i = 0; i < headVec.size(); i++)
	{
		vName = "Head" + to_string(i);
		fs << "{:" << vName << headVec[i] << "}";

	}
	fs << "]";
	*/
	//yaml << "Frames" << "[";
	//writeOutVarInline<int>(fs, testVar, "testVar");
	
	/* Timer.start can be used as reset 
	Timer expTimer;
	expTimer.start();
	while (1)
	{
		int timeInSec = expTimer.getElapsedTimeInSec();
		cout << "Time (s): " << timeInSec << endl;
		if (timeInSec > 10)
			expTimer.start();

	}
	*/

	/* Test camera function 
	
	Timer expTimer;
	expTimer.start();

	CameraData cams;
	cams.initialize();

	while (cams.grabPylonImg())
	{
		cout << "Time (s) : " << expTimer.getElapsedTimeInSec() << endl;
#ifdef PYLON_WIN_BUILD
		// Display the grabbed image.
		Pylon::DisplayImage(1, cams.ptrGrabResult);
#endif

	}
	*/


	




	// Enquire how many cameras to use
	// and enter filenames respectively

	/*
	try{ // Handle with missing frames of Pylon cameras
		ExperimentData myExp;
		myExp.initialize();
		myExp.prepareBgImg();
		myExp.runOLexp(); // run operant learning experiment		
	}	
	catch (const GenericException &e)
	{
		// Error handling
		cerr << "An exception occurred." << endl
			<< e.GetDescription() << endl;
	}
	*/
}