#include "shader.hpp"

#include <extern/glad/glad.h>
#include <extern/glm/gtc/type_ptr.hpp>

#include <stdexcept>
#include <iostream>

using namespace TerrainRendering;

void getShaderCompilationErrors(unsigned int shader, std::string type) {
	int success;
    char infoLog[512];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        throw std::runtime_error(type + " shader compilation failed\n" + std::string(infoLog));
    }
}

ShaderComponent::ShaderComponent(std::string source, unsigned int shaderType): type(shaderType) {
	auto cStrSource = source.c_str();

	id = glCreateShader(type);
	glShaderSource(id, 1, &cStrSource, NULL);
	
	glCompileShader(id);

	#ifndef NDEBUG
	try {
	#endif
	
		getShaderCompilationErrors(id, type == GL_FRAGMENT_SHADER ? "Fragment" : type == GL_VERTEX_SHADER ? "Vertex" : type == GL_GEOMETRY_SHADER ? "Geometry" : type == GL_TESS_CONTROL_SHADER ? "Tessellation control" : type == GL_TESS_EVALUATION_SHADER ? "Tessellation evaluation" : "Unknown");
	
	#ifndef NDEBUG
	} catch (std::exception e) {
		std::cout << e.what() << "\n";
		throw e;
	}
	#endif
}

ShaderComponent::~ShaderComponent() noexcept {
	glDeleteShader(id);
}

ShaderProgram::ShaderProgram() {
	id = glCreateProgram();
}

ShaderProgram::~ShaderProgram() noexcept {
	glDeleteProgram(id);
}

void ShaderProgram::attachShader(ShaderComponent shader) {
	glAttachShader(id, shader.id);
}

void getProgramLinkErrors(unsigned int id) {
	int success;
    char infoLog[1024];
	glGetProgramiv(id, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(id, 1024, NULL, infoLog);
        throw std::runtime_error("Shader program failed to link\n" + std::string(infoLog));
    }
}

void ShaderProgram::link() {
	glLinkProgram(id);

	#ifndef NDEBUG
	try {
	#endif
	
		getProgramLinkErrors(id);
	
	#ifndef NDEBUG
	} catch (std::exception e) {
		std::cout << e.what() << "\n";
		throw e;
	}
	#endif
}


void ShaderProgram::bind() {
	glUseProgram(id);
}

void ShaderProgram::unbind() {
	glUseProgram(0);
}

void ShaderProgram::setUniform(std::string location, int x) {
	glUniform1i(glGetUniformLocation(id, location.c_str()), x);
}
void ShaderProgram::setUniform(std::string location, float x) {
	glUniform1f(glGetUniformLocation(id, location.c_str()), x);
}
void ShaderProgram::setUniform(std::string location, glm::vec2 x) {
	glUniform2fv(glGetUniformLocation(id, location.c_str()), 1, glm::value_ptr(x));
}
void ShaderProgram::setUniform(std::string location, glm::vec3 x) {
	glUniform3fv(glGetUniformLocation(id, location.c_str()), 1, glm::value_ptr(x));
}
void ShaderProgram::setUniform(std::string location, glm::mat3 x) {
	glUniformMatrix3fv(glGetUniformLocation(id, location.c_str()), 1, GL_FALSE, glm::value_ptr(x));
}
void ShaderProgram::setUniform(std::string location, glm::mat4 x) {
	glUniformMatrix4fv(glGetUniformLocation(id, location.c_str()), 1, GL_FALSE, glm::value_ptr(x));
}

std::string TerrainRendering::loadShaderSource(std::string inputPath) {
    std::ifstream input(inputPath);
	if (!input) {
		throw std::runtime_error("Failed to import shader source from file: " + inputPath);
	}

    std::string inputText = "";

    for (std::string line; getline(input, line);) {
        inputText = inputText + "\n" + line;
    }

    inputText += "\0";

    input.close();
    input.clear();

    return inputText;
}