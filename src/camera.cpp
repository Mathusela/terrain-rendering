#include "camera.hpp"

#include <iostream>

#include <extern/glm/gtc/matrix_transform.hpp>

using namespace TerrainRendering;

void Camera::updateViewMatrix() {
	viewMatrix = glm::lookAt(position, position+forward, {0.0, 1.0, 0.0});
}

glm::mat4 Camera::getViewMatrix() {
	return viewMatrix;
}

glm::mat4 Camera::getProjectionMatrix() {
	return projectionMatrix;
}

glm::vec3 Camera::getPosition() {
	return position;
}

void Camera::setPosition(glm::vec3 newPosition) {
	position = newPosition;
	updateViewMatrix();
}

glm::vec3 Camera::getForward() {
	return forward;
}

void Camera::setForward(glm::vec3 newForward) {
	forward = glm::normalize(newForward);
	updateViewMatrix();
}

Camera::Camera(glm::vec3 position, glm::vec3 forward, float near, float far): near(near), far(far) {
	setPosition(position);
	setForward(forward);
}

glm::mat4 PerspectiveCamera::generateProjectionMatrix() {
	return glm::perspective<float>(fov, (float)width/height, near, far);
}

PerspectiveCamera::PerspectiveCamera(glm::vec3 position, glm::vec3 forward, float fov, float near, float far, float WIDTH, float HEIGHT): fov(fov), width(WIDTH), height(HEIGHT), Camera(position, forward, near, far) {
	projectionMatrix = generateProjectionMatrix();
}

glm::mat4 OrthographicCamera::generateProjectionMatrix() {
	return glm::ortho(left, right, bottom, top, near, far);
}

OrthographicCamera::OrthographicCamera(glm::vec3 position, glm::vec3 forward, float left, float right, float bottom, float top, float near, float far): left(left), right(right), bottom(bottom), top(top), Camera(position, forward, near, far) {
	projectionMatrix = generateProjectionMatrix();
}

void FrustumFittingCamera::updateFittingFrustum() {
	glm::mat4 inv = glm::inverse(fittingCamera->getProjectionMatrix() * fittingCamera->getViewMatrix());

	for (int x=0; x<2; x++) {
		for (int y=0; y<2; y++) {
			for (int z=0; z<2; z++) {
				glm::vec4 v = inv * glm::vec4((x*2.0)-1.0, (y*2.0)-1.0, (z*2.0)-1.0, 1.0);
				frustumPoints[x*4+y*2+z] = v / v.w;
			}
		}
	}

	updateViewMatrix();
	projectionMatrix = generateProjectionMatrix();
}

void FrustumFittingCamera::updateViewMatrix() {
	glm::vec3 center {0.0, 0.0, 0.0};

	for (const glm::vec4& v : frustumPoints)
		center += glm::vec3(v);
	center /= frustumPoints.size();

	viewMatrix = glm::lookAt(center + *fittingDir, center, {0.0, 1.0, 0.0});
}

glm::mat4 FrustumFittingCamera::generateProjectionMatrix() {
	float minX = std::numeric_limits<float>::max();
	float maxX = std::numeric_limits<float>::lowest();
	float minY = std::numeric_limits<float>::max();
	float maxY = std::numeric_limits<float>::lowest();
	float minZ = std::numeric_limits<float>::max();
	float maxZ = std::numeric_limits<float>::lowest();

	for (const glm::vec4& v : frustumPoints) {
		const auto lightSpace = viewMatrix * v;
		minX = std::min(minX, lightSpace.x);
		maxX = std::max(maxX, lightSpace.x);
		minY = std::min(minY, lightSpace.y);
		maxY = std::max(maxY, lightSpace.y);
		minZ = std::min(minZ, lightSpace.z);
		maxZ = std::max(maxZ, lightSpace.z);
	}

	if (minZ < 0)
		minZ *= pullNear;
	else
		minZ /= pullNear;
	if (maxZ < 0)
		maxZ /= pushFar;
	else
		maxZ *= pushFar;

	return glm::ortho(minX, maxX, minY, maxY, minZ, maxZ);
}

FrustumFittingCamera::FrustumFittingCamera(Camera* fittingCamera, glm::vec3* fittingDir, float pullNear, float pushFar) : fittingCamera(fittingCamera), fittingDir(fittingDir), pullNear(pullNear), pushFar(pushFar), Camera(glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 0.0, 1.0), 0.1f, 100.0f) {
	updateFittingFrustum();
}