#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;

int loadSHADER(string vertex_file_path, string fragment_file_path) {

	// Check if OpenGL context is current before creating shaders
	if (glGetString(GL_VERSION) == nullptr) {
		std::cerr << "ERROR: OpenGL context is not current before shader creation!" << std::endl;
		std::cerr << "Make sure glfwMakeContextCurrent(window); and glewInit(); are called before loadSHADER." << std::endl;
		return 0;
	}
	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if (VertexShaderStream.is_open()) {
		std::stringstream sstr;
		sstr << VertexShaderStream.rdbuf();
		VertexShaderCode = sstr.str();
		VertexShaderStream.close();
	}
	else {
		printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_file_path);
		getchar();
		return 0;
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if (FragmentShaderStream.is_open()) {
		std::stringstream sstr;
		sstr << FragmentShaderStream.rdbuf();
		FragmentShaderCode = sstr.str();
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Open shader log file for writing
	std::string shaderLogPath = "shader_log.txt";
	std::ofstream shaderLog(shaderLogPath, std::ios::app);
	if (!shaderLog.is_open()) {
		std::cerr << "ERROR: Could not open " << shaderLogPath << " for writing." << std::endl;
	} else {
		shaderLog << "--- Shader Log Start ---" << std::endl;
		shaderLog.flush();
	}

	// Compile Vertex Shader
	if (shaderLog.is_open()) {
		shaderLog << "Compiling shader : " << vertex_file_path << std::endl;
		shaderLog.flush();
	}
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0 && shaderLog.is_open()) {
		std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		shaderLog << &VertexShaderErrorMessage[0] << std::endl;
		shaderLog.flush();
	}

	// Compile Fragment Shader
	if (shaderLog.is_open()) {
		shaderLog << "Compiling shader : " << fragment_file_path << std::endl;
		shaderLog.flush();
	}
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0 && shaderLog.is_open()) {
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		shaderLog << &FragmentShaderErrorMessage[0] << std::endl;
		shaderLog.flush();
	}

	// Link the program
	if (shaderLog.is_open()) {
		shaderLog << "Linking program" << std::endl;
		shaderLog.flush();
	}
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0 && shaderLog.is_open()) {
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		shaderLog << &ProgramErrorMessage[0] << std::endl;
		shaderLog.flush();
	}

	if (shaderLog.is_open()) {
		shaderLog << "--- Shader Log End ---" << std::endl;
		shaderLog.flush();
		shaderLog.close();
	}

	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}