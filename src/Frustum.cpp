#include "Frustum.h"

#include <glm/gtc/matrix_access.hpp>
#include "spdlog/spdlog.h"

void FrustumPlanes::applyTransform(const glm::mat4& transform)
{
	topFace.applyTransform(transform);
	bottomFace.applyTransform(transform);
	rightFace.applyTransform(transform);
	leftFace.applyTransform(transform);
	farFace.applyTransform(transform);
	nearFace.applyTransform(transform);
}

void Frustum::setProjectionMatrix(const glm::mat4& proj)
{
	projectionMatrix = proj;

	glm::vec4 rowX, row4 = glm::row(proj, 3);
	
	Plane* planePtrs[] = {
		&planes.leftFace,
		&planes.rightFace,
		&planes.bottomFace,
		&planes.topFace,
		&planes.nearFace,
		&planes.farFace
	};

	for (int i = 0; i < 3; ++i)
	{
		rowX = glm::row(proj, i);
		for (int j = 0; j < 2; ++j)
		{
			float sign = (j == 0) ? 1.0f : -1.0f;

			glm::vec4 combined = row4 + sign * rowX;
			glm::vec3 normal = glm::vec3(combined);
			float length = glm::length(normal);

			Plane& plane = *planePtrs[i * 2 + j];
			plane.normal = glm::normalize(normal);
			plane.distance = -combined.w / length;
		}
	}

}