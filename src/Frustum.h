#pragma once

#include <glm/glm.hpp>

struct Plane
{
    glm::vec3 normal = { 0.0f, 1.0f, 0.0f };
    float     distance = 0.0f;

    Plane() = default;

    Plane(const glm::vec3& p1, const glm::vec3& norm)
        : normal(glm::normalize(norm)),
        distance(glm::dot(normal, p1))
    {
    }

    float getSignedDistanceToPlane(const glm::vec3& point) const
    {
        return glm::dot(normal, point) - distance;
    }

	void applyTransform(const glm::mat4& transform)
	{
		glm::vec3 point = normal * distance;
		point = glm::vec3(transform * glm::vec4(point, 1.0f));

		glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(transform)));
		normal = glm::normalize(normalMatrix * normal);
		distance = glm::dot(normal, point);
	}
};

struct FrustumPlanes
{
	Plane topFace;
	Plane bottomFace;
	Plane rightFace;
	Plane leftFace;
	Plane farFace;
	Plane nearFace;

	void applyTransform(const glm::mat4& transform);
};

struct Frustum
{
public:
	Frustum(const glm::mat4& projMatrix = glm::mat4(1.0f))
	{
        setProjectionMatrix(projMatrix);
	}

	const FrustumPlanes& getPlanes() const { return planes; }
	const glm::mat4& getProjectionMatrix() const { return projectionMatrix; }
	void setProjectionMatrix(const glm::mat4& proj);

private:
	FrustumPlanes planes;
    glm::mat4 projectionMatrix;
};