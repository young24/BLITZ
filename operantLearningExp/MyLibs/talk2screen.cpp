// Include files to use OpenCV API.
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/video.hpp>


// 
#include "talk2screen.h"
#include "fishAnalysis.h"

// Include files to use the OpenGL API
#include "../OpenGL/shader_s.h"
#include "../OpenGL/stb_image.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Include standard libraries
#include <iostream>
#include <cstdio>
#include <ctime>
#include <string>

using namespace std;

int screenData_init(ScreenData *myScreen)
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_AUTO_ICONIFY, GL_FALSE);
	int count;
	myScreen->monitors = glfwGetMonitors(&count);
	myScreen->mode = glfwGetVideoMode(myScreen->monitors[1]);
	// glfw window creation
	// --------------------
	myScreen->window = glfwCreateWindow(myScreen->mode->width, myScreen->mode->height, "LearnOpenGL", myScreen->monitors[1], NULL);
	cout << "Screen width: " << myScreen->mode->width << endl;
	cout << "Screen height: " << myScreen->mode->height << endl;
	if (myScreen->window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(myScreen->window);
	
	
	return 0;
}

void set_up_buffer_objects(unsigned int* VBOref, unsigned int* VAOref, unsigned int* EBOref, ScreenData* myScreen)
{
	glGenVertexArrays(1, VAOref);
	glGenBuffers(1, VBOref);
	glGenBuffers(1, EBOref);

	glBindVertexArray(*VAOref);

	glBindBuffer(GL_ARRAY_BUFFER, *VBOref);
	glBufferData(GL_ARRAY_BUFFER, sizeof(myScreen->vertices), myScreen->vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *EBOref);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(myScreen->indices), myScreen->indices, GL_STATIC_DRAW);

}

void load_texture(unsigned int* texture, const char *fileName)
{
	// load checkerboard pattern
	glGenTextures(1, texture);
	glBindTexture(GL_TEXTURE_2D, *texture); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
											// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load image, create texture and generate mipmaps
	int width, height, nrChannels;
	unsigned char *data = stbi_load(fileName, &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);

}

void set_ratio_for_texture(float ratio[], int n)
{// show the nth texture
	int len = 2*sizeof(ratio) / sizeof(ratio[0]);
	for (int i = 0; i < len; i++)
	{
		if (i != n)
			ratio[i] = 0;
		else
			ratio[i] = 1;
	}

}

void render_texture(Shader* ourShader, ScreenData* myScreen, unsigned int VAO,float* ratio)
{
	
	/* Render here */
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);// black and opaque
	glClear(GL_COLOR_BUFFER_BIT);

	ourShader->use();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, myScreen->texture0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, myScreen->texture1);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, myScreen->texture2);


	// update the uniform color
	int vertexRatioLocation = glGetUniformLocation(ourShader->ID, "ratio");

	
	glUniform4f(vertexRatioLocation, ratio[0],ratio[1],ratio[2],ratio[3]);
	
	
	// render container
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	/* Swap front and back buffers */
	glfwSwapBuffers(myScreen->window);

	/* Poll for and process events */
	glfwPollEvents();

	
}

void give_visual_stimulus(int timeInSec, FishData* myFish)
{
	// operant learning is both time-dependent and position-dependent

}

void give_visual_stimulus(int timeInSec)
{
	// classical conditioning is time-dependent
	if (timeInSec % 10 < 5)
	{// to give pattern 1

	}
	else 
	{// to give pattern 2

	}
}

void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed(by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}