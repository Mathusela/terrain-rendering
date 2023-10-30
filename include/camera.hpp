#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <extern/glm/glm.hpp>

#include <array>

namespace TerrainRendering {

class Camera {
protected:
	glm::vec3 position;
	glm::vec3 forward;
	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;

	float near, far;
	
	virtual void updateViewMatrix();
	virtual glm::mat4 generateProjectionMatrix() = 0;

public:
	Camera(glm::vec3 position, glm::vec3 forward, float near, float far);

	glm::mat4 getViewMatrix();
	glm::mat4 getProjectionMatrix();
	glm::vec3 getPosition();
	void setPosition(glm::vec3 newPosition);
	glm::vec3 getForward();
	void setForward(glm::vec3 newForward);
};

class PerspectiveCamera : public Camera {
	float fov;
	float width, height;

	glm::mat4 generateProjectionMatrix() override;
public:
	PerspectiveCamera(glm::vec3 position, glm::vec3 forward, float fov, float near, float far, float WIDTH, float HEIGHT);
};

class OrthographicCamera : public Camera {
	float left, right, top, bottom;

	glm::mat4 generateProjectionMatrix() override;
public:
	OrthographicCamera(glm::vec3 position, glm::vec3 forward, float left, float right, float bottom, float top, float near, float far);
};

class FrustumFittingCamera : public Camera {
	Camera* fittingCamera;
	glm::vec3* fittingDir;
	std::array<glm::vec4, 8> frustumPoints;

	float pullNear, pushFar;

	glm::mat4 generateProjectionMatrix() override;
	void updateViewMatrix() override;
public:
	void updateFittingFrustum();

	FrustumFittingCamera(Camera* fittingCamera, glm::vec3* fittingDir, float pullNear, float pushFar);
};

}	// namespace TerrainRendering

#endif