#include "CameraComponent.h"

void CameraComponent::updateProjectionMatrix() {
	glm::mat4 projectionMatrix = glm::mat4(1.0f);
	if (projectionType == ProjectionType::PERSPECTIVE) {
		float tanHalfFov = tan(perspective.fov / 2.0f);
		float halfHeight, halfWidth;
		if (fovSizeAxis == FovSizeAxis::VERTICAL) {
			halfHeight = tanHalfFov * nearPlane;
			halfWidth = halfHeight * aspectRatio;
		}
		else {
			halfWidth = tanHalfFov * nearPlane;
			halfHeight = halfWidth / aspectRatio;
		}

		float offsetX = -screenOffset.x * halfWidth;
		float offsetY = screenOffset.y * halfHeight;

		projectionMatrix = glm::frustum(
			-halfWidth + offsetX,
			halfWidth + offsetX,
			-halfHeight + offsetY,
			halfHeight + offsetY,
			nearPlane,
			farPlane
		);
	}
	else if (projectionType == ProjectionType::ORTHOGRAPHIC) {
		float halfHeight, halfWidth;
		if (fovSizeAxis == FovSizeAxis::VERTICAL) {
			halfHeight = orthographic.size / 2.0f;
			halfWidth = halfHeight * aspectRatio;
		}
		else {
			halfWidth = orthographic.size / 2.0f;
			halfHeight = halfWidth / aspectRatio;
		}

		float offsetX = -screenOffset.x * halfWidth;
		float offsetY = screenOffset.y * halfHeight;

		projectionMatrix = glm::ortho(
			-halfWidth + offsetX,
			halfWidth + offsetX,
			-halfHeight + offsetY,
			halfHeight + offsetY,
			nearPlane,
			farPlane
		);
	}

	camera.getFrustum().setProjectionMatrix(projectionMatrix);
	dirty = false;
}