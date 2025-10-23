#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>
using namespace std;

#include <stdlib.h>
#include <string.h>

#include <GL/glew.h>

#include "ShaderManager.h"

/***********************************************************
 *  LoadShaders()
 *
 *  This method is called to load the shader data from 
 *  external GLSL compatible files.
 ***********************************************************/
GLuint ShaderManager::LoadShaders(const char * vertex_file_path,const char * fragment_file_path){

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	// DEBUG: Confirm shader file exists at runtime
	std::ifstream testStream(vertex_file_path);
	if (!testStream.is_open()) {
		std::cerr << "DEBUG: Shader file NOT found at runtime: " << vertex_file_path << std::endl;
	}
	else {
		std::cerr << "DEBUG: Shader file FOUND at runtime: " << vertex_file_path << std::endl;
		testStream.close();
	}

	// Proceed to open the file for actual reading
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if (VertexShaderStream.is_open()) {
		std::stringstream sstr;
		sstr << VertexShaderStream.rdbuf();
		VertexShaderCode = sstr.str();
		VertexShaderStream.close();
	}else{
		printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_file_path);
		getchar();
		return 0;
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::stringstream sstr;
		sstr << FragmentShaderStream.rdbuf();
		FragmentShaderCode = sstr.str();
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;


	// Compile Vertex Shader
	printf("Compiling shader : %s...", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	GLint success;
	GLchar infoLog[1024];
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &success);
	if (!success) {
	    glGetShaderInfoLog(VertexShaderID, 1024, NULL, infoLog);
	    std::cerr << "ERROR::SHADER_COMPILATION_ERROR of type: "
	              << "VERTEX"
	              << "\n" << infoLog << std::endl;
	}

	printf("success\n");

	// Compile Fragment Shader
	printf("Compiling shader : %s...", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		printf("\n%s\n", &FragmentShaderErrorMessage[0]);
	}

	printf("success\n");

	// Link the program
	printf("Linking shader program...");
	GLuint ProgramID = glCreateProgram();
	m_programID = ProgramID;
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 1 ){
		std::vector<char> ProgramErrorMessage(InfoLogLength+1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("\n%s\n", &ProgramErrorMessage[0]);
	}

	// Activate the shader program to set uniforms
	glUseProgram(ProgramID);

	// Enable texture and lighting in the fragment shader
	glUniform1i(glGetUniformLocation(ProgramID, "bUseTexture"), GL_TRUE);
	glUniform1i(glGetUniformLocation(ProgramID, "bUseLighting"), GL_FALSE);

	printf("success\n");
	
	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);
	
	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}


