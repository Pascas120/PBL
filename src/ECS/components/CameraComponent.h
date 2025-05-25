#pragma once

#include <glm/glm.hpp>

#include "ECS/EntityManager.h"
#include "Camera.h"

struct CameraComponent {
	Camera camera;
	EntityID id = (EntityID)-1;

	bool dirty = true;

	float aspectRatio = 16.0f / 9.0f;

	float nearPlane = 0.1f;
	float farPlane = 1000.0f;

	glm::vec2 screenOffset = { 0.0f, 0.0f };

	enum class ProjectionType {
		PERSPECTIVE,
		ORTHOGRAPHIC
	} projectionType = ProjectionType::PERSPECTIVE;

	struct PerspectiveParameters {
		float fov = glm::radians(45.0f);
	} perspective;

	struct OrthographicParameters {
		float size = 5.0f;
	} orthographic;

	enum class FovSizeAxis {
		VERTICAL,
		HORIZONTAL
	} fovSizeAxis = FovSizeAxis::VERTICAL;

	void updateProjectionMatrix();
};