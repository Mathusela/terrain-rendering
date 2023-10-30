#ifndef SHADER_HPP
#define SHADER_HPP

#include <extern/glm/glm.hpp>

#include <fstream>
#include <string>

namespace TerrainRendering {

std::string loadShaderSource(std::string inputPath);

class ShaderComponent {
	unsigned int id;
	unsigned int type;

public:
	ShaderComponent(std::string source, unsigned int shaderType);
	~ShaderComponent() noexcept;

	friend class ShaderProgram;
};

class ShaderProgram {
	unsigned int id;

public:
	ShaderProgram();
	~ShaderProgram() noexcept;

	void attachShader(ShaderComponent shader);
	void link();

	void bind();
	static void unbind();

	void setUniform(std::string location, int x);
	void setUniform(std::string location, float x);
	void setUniform(std::string location, glm::vec2 x);
	void setUniform(std::string location, glm::vec3 x);
	void setUniform(std::string location, glm::mat3 x);
	void setUniform(std::string location, glm::mat4 x);
};

}	// namespace TerrainRendering

#endif