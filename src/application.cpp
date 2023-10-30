#include "application.hpp"

#include "debug.hpp"
#include "texture.hpp"
#include "framebuffer.hpp"
#include "screenquad.hpp"

#include <extern/glm/glm.hpp>
#include <extern/glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <extern/glm/gtx/string_cast.hpp>

using namespace TerrainRendering;

Camera* mouseCallbackCamera;
void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    const float sensitivity = 0.043f;
	
	static float lastx = 1100 / 2;
	static float lasty = 700 / 2;
	static float yaw = 90.0f;
	static float pitch = 0.0f;
	
	float xoffset = xpos - lastx;
    float yoffset = lasty - ypos;
    lastx = xpos;
    lasty = ypos;

    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    mouseCallbackCamera->setForward(glm::normalize(direction));
}

// Global to allow toggling wireframe before deferred lighting pass
bool wireframe = false;

void cameraController(Camera& camera, GLFWwindow* window, double deltaTime) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE)) glfwSetWindowShouldClose(window, true);

	glm::vec3 offset = {0.0, 0.0, 0.0};

	float speed;
	if (glfwGetKey(window, GLFW_KEY_LEFT_ALT)) speed = 0.5f * deltaTime;
	else speed = 2.0f * deltaTime;
	
	glm::vec3 forwardNoVertRot = glm::normalize(camera.getForward() * glm::vec3(1.0, 0.0, 1.0));
	if (glfwGetKey(window, GLFW_KEY_W)) offset += forwardNoVertRot;
	if (glfwGetKey(window, GLFW_KEY_S)) offset += -forwardNoVertRot;
	glm::vec3 up = {0.0, 1.0, 0.0};
	glm::vec3 right = glm::cross(camera.getForward(), up);
	if (glfwGetKey(window, GLFW_KEY_A)) offset += -right;
	if (glfwGetKey(window, GLFW_KEY_D)) offset += right;
	
	if (glfwGetKey(window, GLFW_KEY_SPACE)) offset += up;
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)) offset += -up;

	static bool wireframePrevPressed = false;
	bool wireframePressed = glfwGetKey(window, GLFW_KEY_F);
	if (wireframePressed && !wireframePrevPressed) {
		if (wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		else glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		wireframe = !wireframe;
		wireframePrevPressed = true;
	}
	if (!wireframePressed) wireframePrevPressed = false;

	auto newPos = camera.getPosition() + (offset.x*offset.x + offset.y*offset.y + offset.z*offset.z != 0 ? glm::normalize(offset) : offset)*speed;
	camera.setPosition(newPos);

	#ifndef FULLSCREEN
	static bool captured = false;
	#else
	static bool captured = true;
	#endif
	static bool capturedPrevPressed = false;
	bool capturedPressed = glfwGetKey(window, GLFW_KEY_TAB);
	if (capturedPressed && !capturedPrevPressed) {
		if (captured) glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		else glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		captured = !captured;
		capturedPrevPressed = true;
	}
	if (!capturedPressed) capturedPrevPressed = false;

	glfwSetCursorPosCallback(window, mouseCallback);
}

Application::Application(): WIDTH(1200), HEIGHT(900), TITLE("Terrain Rendering"), SHADOW_WIDTH(8192), SHADOW_HEIGHT(8192) {
	std::cout << "Created Application\n";

	glfwInit();

	// ----- Setup window -----
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	#ifdef MSAA
	glfwWindowHint(GLFW_SAMPLES, 4);
	#endif

	#ifndef FULLSCREEN
	window = glfwCreateWindow(WIDTH, HEIGHT, TITLE, NULL, NULL);
	#else
	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	WIDTH = mode->width;
	HEIGHT = mode->height;
	window = glfwCreateWindow(WIDTH, HEIGHT, TITLE, glfwGetPrimaryMonitor(), NULL);
	#endif

	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);	// VSYNC OFF

	#ifdef FULLSCREEN
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	#endif
	// ------------------------

	// ----- Load OpenGL -----
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	// -----------------------

	// ----- Debug -----
	#ifndef NDEBUG
	enableDebug();
	#endif
	// -----------------

	// ----- Camera -----
	camera = new PerspectiveCamera(glm::vec3(0.0, 25.0, 0.0), glm::vec3(0.0, 0.0, 1.0), 45.0f, 0.01f, 100.0f, WIDTH, HEIGHT);
	mouseCallbackCamera = camera;

	// shadowMapCamera = new OrthographicCamera(glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 0.0, 1.0), -shadowMapSize, shadowMapSize, -shadowMapSize, shadowMapSize, 0.0f, shadowMapSize*2.0f);
	shadowMapCamera = new FrustumFittingCamera(camera, &sunDirection, 3.0, 1.0);
	// ------------------

	// ----- Initialize shaders -----
	terrainQuadProgram = new ShaderProgram;
	ShaderComponent terrainVert(loadShaderSource("../../resources/shaders/terrain/vert.vs"), GL_VERTEX_SHADER);
	ShaderComponent terrainFrag(loadShaderSource("../../resources/shaders/terrain/frag.fs"), GL_FRAGMENT_SHADER);
	ShaderComponent terrainTessControl(loadShaderSource("../../resources/shaders/terrain/tess-control.tesc"), GL_TESS_CONTROL_SHADER);
	ShaderComponent terrainTessEval(loadShaderSource("../../resources/shaders/terrain/tess-eval.tese"), GL_TESS_EVALUATION_SHADER);
	terrainQuadProgram->attachShader(terrainVert);
	terrainQuadProgram->attachShader(terrainFrag);
	terrainQuadProgram->attachShader(terrainTessControl);
	terrainQuadProgram->attachShader(terrainTessEval);
	terrainQuadProgram->link();

	gBufferProgram = new ShaderProgram;
	ShaderComponent gBufferVert(loadShaderSource("../../resources/shaders/gbuffer/vert.vs"), GL_VERTEX_SHADER);
	ShaderComponent gBufferFrag(loadShaderSource("../../resources/shaders/gbuffer/frag.fs"), GL_FRAGMENT_SHADER);
	gBufferProgram->attachShader(gBufferVert);
	gBufferProgram->attachShader(gBufferFrag);
	gBufferProgram->link();

	terrainShadowMapProgram = new ShaderProgram;
	ShaderComponent terrainShadowMapFrag(loadShaderSource("../../resources/shaders/shadowmap/terrain/frag.fs"), GL_FRAGMENT_SHADER);
	terrainShadowMapProgram->attachShader(terrainVert);
	terrainShadowMapProgram->attachShader(terrainShadowMapFrag);
	terrainShadowMapProgram->attachShader(terrainTessControl);
	terrainShadowMapProgram->attachShader(terrainTessEval);
	terrainShadowMapProgram->link();
	// ------------------------------

	// ----- Create vertex arrays -----
	size_t numVertexArrays = sizeof(vertexArrays)/sizeof(unsigned int);
	glCreateVertexArrays(numVertexArrays, (unsigned int*)&vertexArrays);
	// --------------------------------

	// ----- Create buffers -----
	size_t numBuffers = sizeof(buffers)/sizeof(unsigned int);
	glCreateBuffers(numBuffers, (unsigned int*)&buffers);
	// --------------------------

	// ----- Create textures -----
	size_t numTextures = sizeof(textures)/sizeof(unsigned int);
	glCreateTextures(GL_TEXTURE_2D, numTextures, (unsigned int*)&textures);
	// ---------------------------

	// ----- Initialize framebuffers -----
	// GBuffer
	gBufferFramebuffer = new Framebuffer;
	Renderbuffer depthRBO(WIDTH, HEIGHT, GL_DEPTH24_STENCIL8);

	glBindTexture(GL_TEXTURE_2D, textures.gBufferColorTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, WIDTH, HEIGHT, 0, GL_RGB, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	glBindTexture(GL_TEXTURE_2D, textures.gBufferPositionDepthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, WIDTH, HEIGHT, 0, GL_RGBA, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, textures.gBufferNormalTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, WIDTH, HEIGHT, 0, GL_RGB, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, textures.gBufferLightSpacePositionTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, WIDTH, HEIGHT, 0, GL_RGB, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	glBindTexture(GL_TEXTURE_2D, 0);

	FramebufferAttachment gBufferColor(textures.gBufferColorTexture, GL_COLOR_ATTACHMENT0);
	FramebufferAttachment gBufferPosition(textures.gBufferPositionDepthTexture, GL_COLOR_ATTACHMENT1);
	FramebufferAttachment gBufferNormal(textures.gBufferNormalTexture, GL_COLOR_ATTACHMENT2);
	FramebufferAttachment gBufferLightSpacePosition(textures.gBufferLightSpacePositionTexture, GL_COLOR_ATTACHMENT3);
	FramebufferAttachment gBufferDepth(&depthRBO, GL_DEPTH_ATTACHMENT);
	
	gBufferFramebuffer->attachComponent(gBufferColor);
	gBufferFramebuffer->attachComponent(gBufferPosition);
	gBufferFramebuffer->attachComponent(gBufferNormal);
	gBufferFramebuffer->attachComponent(gBufferLightSpacePosition);
	gBufferFramebuffer->attachComponent(gBufferDepth);

	gBufferFramebuffer->setDrawAttachments({GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3});
	
	// Shadow map
	shadowMapFramebuffer = new Framebuffer;

	glBindTexture(GL_TEXTURE_2D, textures.shadowMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	FramebufferAttachment shadowMapAttachment(textures.shadowMapTexture, GL_DEPTH_ATTACHMENT);

	shadowMapFramebuffer->attachComponent(shadowMapAttachment);
	shadowMapFramebuffer->setDrawAttachments({GL_NONE});
	// -----------------------------------

	// ----- Bind images to textures -----
	auto dimensions = load_image_to_texture_UNSIGNED_SHORT(textures.heightmapTexture, "../../resources/heightmaps/mountain_heightmap_4k.png");
	heightmapWidth = dimensions.width; heightmapHeight = dimensions.height;
	std::cout << heightmapWidth << ", " << heightmapHeight << "\n";
	load_image_to_texture_UNSIGNED_BYTE(textures.terrainDiffuseTexture, "../../resources/heightmaps/mountain_diffuse_4k.png");
	// -----------------------------------

	// ----- Initialize objects -----
	initializeTerrainQuad(50, 50);
	fillScreenQuadVAO(vertexArrays.screenQuadVAO);

	sunDirection = glm::vec3(1.0, 0.3, 0.0);
	// ------------------------------
}

Application::~Application() noexcept {
	std::cout << "Destroyed Application\n";

	delete terrainQuadProgram;
	delete gBufferProgram;
	delete terrainShadowMapProgram;

	delete camera;
	delete shadowMapCamera;

	delete gBufferFramebuffer;
	delete shadowMapFramebuffer;

	size_t numVertexArrays = sizeof(vertexArrays)/sizeof(unsigned int);
	glDeleteVertexArrays(numVertexArrays, (unsigned int*)&vertexArrays);

	size_t numBuffers = sizeof(buffers)/sizeof(unsigned int);
	glDeleteBuffers(numBuffers, (unsigned int*)&buffers);

	size_t numTextures = sizeof(textures)/sizeof(unsigned int);
	glDeleteTextures(numTextures, (unsigned int*)&textures);

	glfwTerminate();
}

void Application::run() {
	std::cout << "RUNNING\n";
	
	glPatchParameteri(GL_PATCH_VERTICES, 4);
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	
	// ========== Generate terrain normal map ==========
	// ----- Texture -----
	glBindTexture(GL_TEXTURE_2D, textures.terrainNormalTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, heightmapHeight, heightmapHeight, 0, GL_RGB, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	// -------------------

	// ----- Framebuffer -----
	Framebuffer fbo;

	FramebufferAttachment textureAttachment(textures.terrainNormalTexture, GL_COLOR_ATTACHMENT0);
	fbo.attachComponent(textureAttachment);
	fbo.setDrawAttachments({GL_COLOR_ATTACHMENT0});
	// -----------------------

	// ----- Shader -----
	ShaderProgram normalMapProgram;
	ShaderComponent vert(loadShaderSource("../../resources/shaders/normalmap/vert.vs"), GL_VERTEX_SHADER);
	ShaderComponent frag(loadShaderSource("../../resources/shaders/normalmap/frag.fs"), GL_FRAGMENT_SHADER);
	normalMapProgram.attachShader(vert);
	normalMapProgram.attachShader(frag);
	normalMapProgram.link();
	// ------------------

	// ----- Draw -----
	glViewport(0, 0, heightmapWidth, heightmapHeight);

	fbo.clear();
	fbo.bind();

	normalMapProgram.bind();
	glBindVertexArray(vertexArrays.screenQuadVAO);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures.heightmapTexture);
	normalMapProgram.setUniform("heightmap", 0);

	normalMapProgram.setUniform("heightmapWidth", heightmapWidth);
	normalMapProgram.setUniform("heightmapHeight", heightmapHeight);

	glDrawArrays(GL_TRIANGLES, 0, 6);
	// ----------------

	// ----- Cleanup -----
	glViewport(0, 0, WIDTH, HEIGHT);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
	ShaderProgram::unbind();
	Framebuffer::unbind();
	// -------------------
	// =================================================

	// Gameloop
	gameloop();
}

void Application::drawTerrain(Camera* camera, ShaderProgram* program, glm::vec3 position, float maxHeight, float quadScale) {
		// ----- Setup -----
		program->bind();
		glBindVertexArray(vertexArrays.terrainQuadVAO);
		// -----------------

		// ----- Textures -----
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textures.heightmapTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, textures.terrainDiffuseTexture);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, textures.terrainNormalTexture);
		program->setUniform("heightmap", 0);
		program->setUniform("diffusemap", 1);
		program->setUniform("normalmap", 2);
		// --------------------

		// ----- Uniforms -----
		program->setUniform("projectionMatrix", camera->getProjectionMatrix());
		program->setUniform("viewMatrix", camera->getViewMatrix());
		program->setUniform("heightmapWidth", heightmapWidth);
		program->setUniform("heightmapHeight", heightmapHeight);

		program->setUniform("offset", position);
		program->setUniform("maxHeight", maxHeight);
		program->setUniform("quadScale", quadScale);

		program->setUniform("lightSpaceMatrix", shadowMapCamera->getProjectionMatrix() * shadowMapCamera->getViewMatrix());

		program->setUniform("cameraPos", camera->getPosition());
		// --------------------

		// ----- Draw -----
		glDrawElements(GL_PATCHES, terrainQuadNumberOfIndicies, GL_UNSIGNED_INT, 0);
		// ----------------
		
		// ----- Cleanup -----
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindVertexArray(0);
		ShaderProgram::unbind();
		// -------------------
}

void Application::gameloop() {
	double deltaTime = 0.0;
	double prevTime = 0.0;

	while (!glfwWindowShouldClose(window)) {
		// ----- Delta time -----
		auto currentTime = glfwGetTime();
		deltaTime = currentTime - prevTime;
		prevTime = currentTime;
		// ----------------------

		// ----- Update sun direction -----
		// auto r = glm::rotate(glm::identity<glm::mat4>(), (float)deltaTime*0.1f, glm::vec3(0.0, 1.0, 0.0)) * glm::vec4(sunDirection.x, sunDirection.y, sunDirection.z, 1.0);
		// sunDirection = glm::vec3(r.x, r.y, r.z);
		// --------------------------------

		// ----- Camera controller -----
		cameraController(*camera, window, deltaTime);
		// ---------------------------

		// ----- Update shadow map camera position -----
		((FrustumFittingCamera*)shadowMapCamera)->updateFittingFrustum();
		// ---------------------------------------------

		// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		// ~~~~~~~~~~~~~~~~~~~~ Draw ~~~~~~~~~~~~~~~~~~~~
		// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 

		// ========== Render to shadow map ==========
		// ----- Setup -----
		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		// -----------------

		// ----- Setup shadow map framebuffer -----
		shadowMapFramebuffer->clear();
		shadowMapFramebuffer->bind();
		// ----------------------------------------

		// ----- Draw -----
		drawTerrain(shadowMapCamera, terrainShadowMapProgram, glm::vec3(0.0, -3.0, 0.0), 60.0f, 100.0f);

		// Shadow caster under terrain because terrain is infinitely thin
		// drawTerrain(shadowMapCamera, terrainShadowMapProgram, glm::vec3(0.0, -3.5, 0.0), 60.0f, 100.0f);
		// drawTerrain(shadowMapCamera, terrainShadowMapProgram, glm::vec3(0.0, 20.0, 5.0), 5.0f, 100.0f/30.0f);
		// ----------------	

		// ----- Cleanup -----
		glViewport(0, 0, WIDTH, HEIGHT);
		// -------------------
		// ==========================================

		// ========== Render scene ==========
		// ----- Setup GBuffer framebuffer -----
		gBufferFramebuffer->clear();
		gBufferFramebuffer->bind();
		// -------------------------------------

		// ----- Draw ground -----
		drawTerrain(camera, terrainQuadProgram, glm::vec3(0.0, -3.0, 0.0), 60.0f, 100.0f);

		// drawTerrain(camera, terrainQuadProgram, glm::vec3(0.0, -3.5, 0.0), 60.0f, 100.0f);
		// drawTerrain(camera, terrainQuadProgram, glm::vec3(0.0, 20.0, 5.0), 5.0f, 100.0f/30.0f);
		// -----------------------
		
		// TODO: Render everything else with MDI
		// ==================================

		// ========== Deferred lighting pass ==========
		// ----- Bind default framebuffer -----
		Framebuffer::unbind();
		// ------------------------------------
		
		// ----- Clear -----
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// -----------------

		// ----- Setup -----
		if (wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		gBufferProgram->bind();
		glBindVertexArray(vertexArrays.screenQuadVAO);
		// -----------------

		// ----- Textures -----
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textures.gBufferColorTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, textures.gBufferPositionDepthTexture);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, textures.gBufferNormalTexture);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, textures.gBufferLightSpacePositionTexture);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, textures.shadowMapTexture);
		gBufferProgram->setUniform("colormap", 0);
		gBufferProgram->setUniform("positiondepthmap", 1);
		gBufferProgram->setUniform("normalmap", 2);
		gBufferProgram->setUniform("lightspacepositionmap", 3);
		gBufferProgram->setUniform("shadowmap", 4);
		// --------------------

		// ----- Uniforms -----
		gBufferProgram->setUniform("inverseProjectionMatrix", glm::inverse(camera->getProjectionMatrix()));
		gBufferProgram->setUniform("inverseViewMatrix", glm::inverse(camera->getViewMatrix()));

		gBufferProgram->setUniform("cameraPos", camera->getPosition());
		gBufferProgram->setUniform("sunDirection", sunDirection);
		// --------------------

		// ----- Draw -----
		glDrawArrays(GL_TRIANGLES, 0, 6);
		// ----------------

		// ----- Cleanup -----
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindVertexArray(0);
		ShaderProgram::unbind();
		if (wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		// -------------------
		// ============================================

		glfwSwapBuffers(window);	// Display rendered image to screen
		glfwPollEvents();			// Poll the operating system
	}
}

void Application::initializeTerrainQuad(unsigned int resolutionX, unsigned int resolutionY) {
	struct Vert {
		float position[3];
		float uv[2];
	};

	struct Quad {
		unsigned int verts[4];
	};

	std::vector<Vert> verts;
	verts.reserve(resolutionX*resolutionY);

	for (unsigned int i=0; i<=resolutionX; i++) 
		for (unsigned int j=0; j<=resolutionY; j++) {
			verts.push_back(Vert {
					{i/(float)resolutionX - 0.5f, 0.0f, j/(float)resolutionY - 0.5f},
					{i/(float)resolutionX, j/(float)resolutionY}
				});
		}
	terrainQuadNumberOfVerts = verts.size();

	std::vector<Quad> indicies;
	indicies.reserve(terrainQuadNumberOfVerts-(resolutionX+1)-(resolutionY));

	for (unsigned int i=0; i<resolutionX*(resolutionY+1)-1; i++) {
		if ((i+1)%(resolutionY+1) == 0) continue;
		indicies.push_back(Quad {{i, i+1, resolutionY+1+i, resolutionY+2+i}});
	}
	terrainQuadNumberOfIndicies = indicies.size()*4;

	glNamedBufferData(buffers.terrainQuadVBO, sizeof(Vert)*terrainQuadNumberOfVerts, &verts[0], GL_STATIC_DRAW);
	glNamedBufferData(buffers.terrainQuadEBO, sizeof(Quad)*indicies.size(), &indicies[0], GL_STATIC_DRAW);


	glVertexArrayVertexBuffer(vertexArrays.terrainQuadVAO, 0, buffers.terrainQuadVBO, 0, sizeof(Vert));
	glVertexArrayElementBuffer(vertexArrays.terrainQuadVAO, buffers.terrainQuadEBO);

	glVertexArrayAttribFormat(vertexArrays.terrainQuadVAO, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vert, position));
	glVertexArrayAttribBinding(vertexArrays.terrainQuadVAO, 0, 0);
	glEnableVertexArrayAttrib(vertexArrays.terrainQuadVAO, 0);

	glVertexArrayAttribFormat(vertexArrays.terrainQuadVAO, 1, 2, GL_FLOAT, GL_FALSE, offsetof(Vert, uv));
	glVertexArrayAttribBinding(vertexArrays.terrainQuadVAO, 1, 0);
	glEnableVertexArrayAttrib(vertexArrays.terrainQuadVAO, 1);
}
