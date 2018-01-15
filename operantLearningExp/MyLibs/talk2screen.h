#ifndef _GUARD_TALK2SCREEN_H
#define _GUARD_TALK2SCREEN_H


#pragma once
// Include files to use the OpenGL API
#include "../OpenGL/shader_s.h"
#include "../OpenGL/stb_image.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>


typedef struct ScreenDataStruct
{
	GLFWmonitor** monitors;
	const GLFWvidmode* mode;
	GLFWwindow* window;
	float vertices[32] = {
		// positions          // colors           // texture coords
		1.0f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
		1.0f, -1.0f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
		-1.0f, -1.0f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
		-1.0f,  1.0f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // top left 
	};
	unsigned int indices[6] = {
		0, 1, 3, // first triangle
		1, 2, 3  // second triangle
	};
	unsigned int texture0;
	unsigned int texture1;
	unsigned int texture2;
	unsigned int texture3;
	
}ScreenData;


int screenData_init(ScreenData *myScreen);
void set_up_buffer_objects(unsigned int*, unsigned int*, unsigned int*, ScreenData* myScreen);
void load_texture(unsigned int* texture, const char *fileName);
void set_ratio_for_texture(float* ratio, int n);
void render_texture(Shader* ourShader, ScreenData* myScreen, unsigned int VAO, float* ratio);
void processInput(GLFWwindow *window);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
#endif // !_GUARD_TALK2SCREEN_H
