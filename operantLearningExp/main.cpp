// Experiment.cpp : Defines the entry point for the console application.
//

// This program should have 4 parts:
// 1. Pylon camera video stream
// 2. OpenCV image processing to get fish position
// 3. Relay control (seiral communication)
// 4. OpenGL (GLFW+glad) to give visual stimuli

// user defined macros
#define COM_NUM 4 // the top electrodes, attached to the outer USB port
#define UsingRelay 1

// Include files to use OpenCV API.
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/video.hpp>

// Include files to use the PYLON API.
#include <pylon/PylonIncludes.h>
#ifdef PYLON_WIN_BUILD
#	include <pylon/PylonGUI.h>
#endif


// Include DIY libraries
#include "Mylibs/fishAnalysis.h"
#include "Mylibs/talk2relay.h"
#include "Mylibs/talk2screen.h"

// Include files to use the OpenGL API
#include "OpenGL/shader_s.h"
#include "OpenGL/stb_image.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Include standard libraries
#include <iostream>
#include <cstdio>
#include <ctime>
#include <Windows.h>
#include <random>
#include <iomanip>

// 
#include "SerialCom/SerialPort.h"


using namespace Pylon;
using namespace cv;
using namespace std;

// Automatically call PylonInitialize and PylonTerminate to ensure the pylon runtime system
// is initialized during the lifetime of this object.
Pylon::PylonAutoInitTerm autoInitTerm;


void mouse_callback(int event, int x, int y, int flag, void* param);
int delimY = -1;




int main()
{
	Rect cropRoi(0, 0, 0, 0);
	clock_t expStartTime = clock();
	random_device rd;//random device
	mt19937 rng(rd()); // random engine used
	uniform_int_distribution<int> uni(15, 45);// seconds
	unsigned char open_C1_moment[] = { 0x00,0x5A,0x54,0x00,0x12,0x01,0x00,0x01,0xC2 }; //open the channel 1 for 0.1 s
	unsigned char open_C2_moment[] = { 0x00,0x5A,0x54,0x00,0x12,0x02,0x00,0x01,0xC3 }; //open the channel 2 for 0.1 s
																					   // open channel 1: 00 5A 54 00 01 01 00 00 B0
																					   // close channel 1: 00 5A 54 00 02 01 00 00 B1
	int baseLineEndTime = 10 * 60; // seconds, default 10 mins
	int trainingEndTime = 25 * 60; // seconds, default 15 mins
	int blackOutEndTime = 28 * 60; // 3 mins blackout
	int testEndTime = 32 * 60; // seconds, 4 mins test
	int testInterval = 30; // seconds
	int blackOutInterval = 0; // blackout interval during training

							  //glfw: initialize and configure
	ScreenData myScreen;

	if (screenData_init(&myScreen)) {
		cout << "Something went wrong !" << endl;
	}

	// glad: load all OpenGL function pointers
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	unsigned int VBO, VAO, EBO;

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(myScreen.vertices), myScreen.vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(myScreen.indices), myScreen.indices, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	// texture coord attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	// load and create a texture 
	// -------------------------
	const char fileName0[] = "Images/shiftedTopRedChecker.jpg";
	const char fileName1[] = "Images/shiftedBottomRedChecker.jpg";
	const char fileName2[] = "Images/black.jpg";
	//const char fileName3[] = "Images/checkerboard.jpg";

	load_texture(&myScreen.texture0, fileName0);
	load_texture(&myScreen.texture1, fileName1);
	load_texture(&myScreen.texture2, fileName2);


	// build and compile our shader zprogram
	// ------------------------------------
	Shader ourShader("OpenGL/shader.vs", "OpenGL/shader.fs");
	// tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
	// -------------------------------------------------------------------------------------------
	ourShader.use(); // don't forget to activate/use the shader before setting uniforms!
					 // either set it manually like so:
					 // set texture ID
	ourShader.setInt("texture0", 0);
	ourShader.setInt("texture1", 1);
	ourShader.setInt("texture2", 2);


	float ratio[4] = { 0 };

	// Create an instant camera object with the camera device found first.
	CInstantCamera camera(CTlFactory::GetInstance().CreateFirstDevice());
	// Print the model name of the camera.
	cout << "Using device " << camera.GetDeviceInfo().GetModelName() << endl;
	// Open the camera before accessing any parameters.
	camera.Open();
	// Get a camera nodemap in order to access camera parameters.
	GenApi::INodeMap& nodemap = camera.GetNodeMap();
	// Create pointers to access the camera Width and Height parameters.
	GenApi::CIntegerPtr width = nodemap.GetNode("Width");
	GenApi::CIntegerPtr height = nodemap.GetNode("Height");
	// Create a pylon ImageFormatConverter object.
	CImageFormatConverter formatConverter;
	// Specify the output pixel format.
	formatConverter.OutputPixelFormat = PixelType_Mono8;
	// Create a PylonImage that will be used to create OpenCV images later.
	CPylonImage pylonImage;
	// The camera device is parameteried with a default configuration which 
	// sets up free-running continuous acquisition.
	camera.StartGrabbing(GrabStrategy_LatestImageOnly);
	// This smart pointer will receive the grab result data.
	CGrabResultPtr ptrGrabResult;
	// Camera.StopGrabbing() is called automatically by the RetrieveResult() method
	// when c_countOfImagesToGrab images have been retrieved.

	FishData myFish;
	myFish.frameSize = Size((int)width->GetValue(), (int)height->GetValue());

	cropRoi = Rect(0, 0, myFish.frameSize.width, myFish.frameSize.height);
	namedWindow("Display", WINDOW_NORMAL);
	namedWindow("Control Panel", 1);

	// TODO: make a gui to control these parameters
	int recordVideo = 0;
	int analysisOn = 0;
	int expStartFlag = 0;
	int giveShockTop = 0;
	int giveShockBottom = 0;
	int ROIon = 0;

	createTrackbar("record", "Control Panel", &recordVideo, 1);
	createTrackbar("analysisOn", "Control Panel", &analysisOn, 1);
	createTrackbar("binThre", "Control Panel", &myFish.threBin, 250);
	createTrackbar("ROIon", "Control Panel", &ROIon, 1);
	createTrackbar("E-shockTop", "Control Panel", &giveShockTop, 1);
	createTrackbar("E-shockBottom", "Control Panel", &giveShockBottom, 1);

	recordVideo = getTrackbarPos("record", "Control Panel");
	analysisOn = getTrackbarPos("analysisOn", "Control Panel");
	myFish.threBin = getTrackbarPos("binThre", "Control Panel");
	createTrackbar("ROIon", "Control Panel", &ROIon, 1);
	giveShockTop = getTrackbarPos("E-shockTop", "Control Panel");
	giveShockBottom = getTrackbarPos("E-shockBottom", "Control Panel");


	CSerialPort myPort;
	if (!myPort.InitPort(COM_NUM))
	{
		std::cout << "initPort fail !" << std::endl;
	}
	else
	{
		std::cout << "initPort success !" << std::endl;
	}
	//HANDLE hCom = initialize_relay(COM_NUM);// handle to relay, to open later

	string fileName;
	cout << "Enter the fileName (date_fishID_expType)" << endl;
	cin >> fileName;
	string videoFileName = "G:/FishExpData/operantLearning/";
	videoFileName += fileName + ".avi";
	VideoWriter cvVideoCreator;
	// last argument means no color.
	string yamlName = "G:/FishExpData/operantLearning/";
	yamlName += fileName + ".yaml";
	FileStorage fs(yamlName, FileStorage::WRITE);

	int timeInSec;
	int timeMSec;
	int lastStiTime = baseLineEndTime; // seconds
	auto interval = 0;// inter-trial interval during training session
	int patternFlag = 1;
	int idxFrame = 0;
	int shockCD = 2;
	double thinkingTime = 2.0; // give fish some time to think
	int lastShockTime = -shockCD;
	vector<Mat> images;
	while (camera.IsGrabbing())
	{
		//Wait for an image and then retrieve it. A timeout of 5000 ms is used.
		camera.RetrieveResult(5000, ptrGrabResult, TimeoutHandling_ThrowException);
		// Image grabbed succesfully?
		if (ptrGrabResult->GrabSucceeded())
		{
			if (waitKey(30) == 27) break;// Esc key
			formatConverter.Convert(pylonImage, ptrGrabResult);
			myFish.openCvImage = Mat(ptrGrabResult->GetHeight(), ptrGrabResult->GetWidth(), CV_8UC1, (uint8_t *)pylonImage.GetBuffer());
			//rot_90_CCW(&myFish);// Rotate image 90 degrees CCW to align with natural view
			myFish.openCvImage.copyTo(myFish.rotImage);

			if (ROIon)
			{
				cropRoi = selectROI("Display", myFish.rotImage);
				cvResizeWindow("Display", cropRoi.width, cropRoi.height);
				myFish.frameSize = cropRoi.size();
				cvVideoCreator.open(videoFileName, CV_FOURCC('D', 'I', 'V', 'X'), 20.0, myFish.frameSize, false);
				delimY = myFish.frameSize.height / 2;
				ROIon = 0;
			}
			crop_image(&myFish, cropRoi);

			// click and crop
			setMouseCallback("Display", mouse_callback);
			// manually give e-shock
			if (giveShockTop)
			{
				myPort.WriteData(open_C1_moment, 9); // top electrodes
				fs << "TopShock" << idxFrame;
				giveShockTop = 0;
			}
			if (giveShockBottom)
			{
				myPort.WriteData(open_C2_moment, 9); // bottom electrodes
				fs << "BottomShock" << idxFrame;
				giveShockBottom = 0;
			}

			// After recording, the experiment is on.
			if (!expStartFlag)
			{
				expStartTime = clock(); //  record experiment start time
			}
			if (recordVideo)
			{
				idxFrame++;
				fs << "FrameNum" << idxFrame;
				updateMean(&images, &myFish, idxFrame);

				expStartFlag = 1;
				timeInSec = (int)((clock() - expStartTime) / CLOCKS_PER_SEC);
				timeMSec = (int)(clock() * 1000 - timeInSec * 1000);
				cout << "time is :" << timeInSec << "(s)" << endl;

				// Initialize the head/center position
				myFish.head.x = -1;
				myFish.head.y = -1;
				myFish.center.x = -1;
				myFish.center.y = -1;
				if (analysisOn)
				{

					get_fish_pos(&myFish);
					if (myFish.head.y > 0)
					{
						circle(myFish.HUDSimg, myFish.head, 6, 255, 2);
						circle(myFish.HUDSimg, myFish.center, 3, 255, 2);
					}
				}

				// Baseline period (0-30 mins)
				if (timeInSec <= baseLineEndTime)
				{
					myFish.patternIdx = 0;// bottom checkerboard
					set_ratio_for_texture(ratio, myFish.patternIdx);// show the 0 th texture
					myFish.lastStiIdx = myFish.patternIdx;
					render_texture(&ourShader, &myScreen, VAO, ratio);
				}
				else if (timeInSec <= trainingEndTime)// Training period
				{
					// set visual stimulus
					if (timeInSec >= lastStiTime + interval + blackOutInterval)
					{
						cout << "Visual Pattern Change" << endl;
						fs << "PatternChange" << idxFrame;
						lastStiTime = timeInSec;
						interval = uni(rng);// set interval as a random number in a range
						if (myFish.lastStiIdx)
						{
							myFish.patternIdx = 0;// top red		
						}
						else
						{
							myFish.patternIdx = 1;// bottom red
						}
						myFish.lastStiIdx = myFish.patternIdx;
						set_ratio_for_texture(ratio, myFish.patternIdx);
						render_texture(&ourShader, &myScreen, VAO, ratio);
					}
					else if ((timeInSec >= lastStiTime + interval) && (timeInSec < lastStiTime + interval + blackOutInterval))// blackout time
					{
						myFish.patternIdx = 2;
						set_ratio_for_texture(ratio, myFish.patternIdx);
						render_texture(&ourShader, &myScreen, VAO, ratio);
					}
					else
					{
						set_ratio_for_texture(ratio, myFish.patternIdx);
						render_texture(&ourShader, &myScreen, VAO, ratio);
					}

					if ((timeInSec > lastShockTime + shockCD) && (timeInSec > lastStiTime + thinkingTime)) // the lastStiTime has updated in first if.
					{
						if (myFish.patternIdx == 1) // pattern ID 1
						{
							if (myFish.head.y > delimY)  // bottom red
							{
								if (myFish.headingAngle > 0)
								{
									myPort.WriteData(open_C2_moment, 9); // bottom electrodes
									fs << "shockOnBottom" << 1;
									lastShockTime = timeInSec;
								}
							}
						}
						else if (myFish.patternIdx == 0) {// pattern ID 0
							if (myFish.head.y > 0)//otherwise, it's not a valid value
							{
								if ((myFish.head.y > 0) && (myFish.head.y < delimY))// top red
								{
									if (myFish.headingAngle < 0)
									{
										myPort.WriteData(open_C1_moment, 9); // top electrodes
										fs << "shockOnTop" << 1;
										lastShockTime = timeInSec;
									}
								}
							}
						}
					}

				}
				else if (timeInSec <= blackOutEndTime)
				{// blackout
					set_ratio_for_texture(ratio, 2);
					render_texture(&ourShader, &myScreen, VAO, ratio);
				}
				else if (timeInSec <= testEndTime)
				{
					patternFlag = myFish.patternIdx;

					if (((timeInSec - trainingEndTime) / testInterval) % 2)
					{
						myFish.patternIdx = 0;// bottom black

						if (abs(patternFlag - myFish.patternIdx))
						{
							patternFlag = 0;
						}
						set_ratio_for_texture(ratio, 0);// show the 0 th texture
						render_texture(&ourShader, &myScreen, VAO, ratio);
					}
					else
					{
						myFish.patternIdx = 1;// bottom black

						if (abs(patternFlag - myFish.patternIdx))
						{
							fs << "Pattern Change" << myFish.patternIdx;
							patternFlag = 0;
						}
						set_ratio_for_texture(ratio, 1);// show the 0 th texture
						render_texture(&ourShader, &myScreen, VAO, ratio);
					}
				}
				else {
					// Experiment ends.
					cout << "Experiment End" << endl;
					exit(0);
				}
				// write out the video
				cvVideoCreator.write(myFish.rawImg);

				// write fish info to yaml
				fs << "PatternIdx" << myFish.patternIdx;
				fs << "Head" << myFish.head;
				fs << "Center" << myFish.center;
				fs << "Tail" << myFish.tail;
				fs << "HeadingAngle" << myFish.headingAngle;
			}
			imshow("Display", myFish.HUDSimg);
		}

	}
	fs.release();
}

void mouse_callback(int event, int x, int y, int flag, void* param)
{
	if (event == cv::EVENT_LBUTTONDOWN) {
		std::cout << "(" << x << ", " << y << ")" << std::endl;
		delimY = y;
	}
}