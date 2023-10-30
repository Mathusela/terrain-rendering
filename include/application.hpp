#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <extern/glad/glad.h>
#include <extern/GLFW/glfw3.h>

#include <extern/glm/glm.hpp>

#include "shader.hpp"
#include "camera.hpp"
#include "framebuffer.hpp"

#include <string>
#include <vector>
#include <array>

// === OPTIONS ===
// #define FULLSCREEN
// #define MSAA
// ===============

namespace TerrainRendering {

class Application {
	int WIDTH;
	int HEIGHT;
	const char TITLE[18];

	int SHADOW_WIDTH;
	int SHADOW_HEIGHT;

	const float shadowMapSize = 100.0;

	glm::vec3 sunDirection;

	GLFWwindow* window;
	Camera* camera;
	Camera* shadowMapCamera;

	unsigned int terrainQuadNumberOfVerts;
	unsigned int terrainQuadNumberOfIndicies;

	int heightmapWidth, heightmapHeight;
	
	struct {
		unsigned int terrainQuadVAO;
		unsigned int screenQuadVAO;
	} vertexArrays;
	struct {
		unsigned int terrainQuadVBO;
		unsigned int terrainQuadEBO;
	} buffers;
	struct {
		unsigned int heightmapTexture;
		unsigned int terrainDiffuseTexture;
		unsigned int terrainNormalTexture;

		unsigned int gBufferColorTexture;
		unsigned int gBufferPositionDepthTexture;
		unsigned int gBufferNormalTexture;
		unsigned int gBufferLightSpacePositionTexture;

		unsigned int shadowMapTexture;
	} textures;

	ShaderProgram* terrainQuadProgram;
	ShaderProgram* gBufferProgram;
	ShaderProgram* terrainShadowMapProgram;

	Framebuffer* gBufferFramebuffer;
	Framebuffer* shadowMapFramebuffer;

	void drawTerrain(Camera* camera, ShaderProgram* program, glm::vec3 position, float maxHeight, float quadScale);
	void gameloop();
	void initializeTerrainQuad(unsigned int resolutionX, unsigned int resolutionY);

public:
	Application();
	~Application() noexcept;

	void run();
};

}	// namespace TerrainRendering

#endif