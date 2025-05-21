#include "BoundingVolumes.h"
#include "Camera.h"
#include "Components.h"

// temporary
#include "glm_serialization.h"


bool BoundingVolume::isOnFrustum(const Frustum& camFrustum) const
{
    return (isOnOrForwardPlane(camFrustum.leftFace) &&
            isOnOrForwardPlane(camFrustum.rightFace) &&
            isOnOrForwardPlane(camFrustum.topFace) &&
            isOnOrForwardPlane(camFrustum.bottomFace) &&
            isOnOrForwardPlane(camFrustum.nearFace) &&
            isOnOrForwardPlane(camFrustum.farFace));
}

// Sphere implementation
SphereBV::SphereBV(const glm::vec3& inCenter, float inRadius)
    : center(inCenter), radius(inRadius) {}

bool SphereBV::isOnOrForwardPlane(const Plane& plane) const
{
    return plane.getSignedDistanceToPlane(center) > -radius;
}

bool SphereBV::isOnFrustum(const Frustum& camFrustum, const Transform& transform) const
{
    const glm::vec3 globalScale = {glm::length(transform.globalMatrix[0]), glm::length(transform.globalMatrix[1]), glm::length(transform.globalMatrix[2])};
    const glm::vec3 globalCenter{ transform.globalMatrix * glm::vec4(center, 1.f) };
    const float maxScale = std::max({globalScale.x, globalScale.y, globalScale.z});
    SphereBV globalSphere(globalCenter, radius * (maxScale * 0.5f));

    return (globalSphere.isOnOrForwardPlane(camFrustum.leftFace) &&
            globalSphere.isOnOrForwardPlane(camFrustum.rightFace) &&
            globalSphere.isOnOrForwardPlane(camFrustum.farFace) &&
            globalSphere.isOnOrForwardPlane(camFrustum.nearFace) &&
            globalSphere.isOnOrForwardPlane(camFrustum.topFace) &&
            globalSphere.isOnOrForwardPlane(camFrustum.bottomFace));
}


nlohmann::json SphereBV::serialize() const
{
	nlohmann::json j;
	j["type"] = "SPHERE";
	j["center"] = center;
	j["radius"] = radius;
	return j;
}

// AABB implementation
AABBBV::AABBBV(const glm::vec3& min, const glm::vec3& max)
    : center((max + min) * 0.5f), extents(max.x - center.x, max.y - center.y, max.z - center.z) {}

AABBBV::AABBBV(const glm::vec3& inCenter, float iI, float iJ, float iK)
    : center(inCenter), extents(iI, iJ, iK) {}

std::array<glm::vec3, 8> AABBBV::getVertices() const
{
    std::array<glm::vec3, 8> vertices;
    vertices[0] = { center.x - extents.x, center.y - extents.y, center.z - extents.z };
    vertices[1] = { center.x + extents.x, center.y - extents.y, center.z - extents.z };
    vertices[2] = { center.x - extents.x, center.y + extents.y, center.z - extents.z };
    vertices[3] = { center.x + extents.x, center.y + extents.y, center.z - extents.z };
    vertices[4] = { center.x - extents.x, center.y - extents.y, center.z + extents.z };
    vertices[5] = { center.x + extents.x, center.y - extents.y, center.z + extents.z };
    vertices[6] = { center.x - extents.x, center.y + extents.y, center.z + extents.z };
    vertices[7] = { center.x + extents.x, center.y + extents.y, center.z + extents.z };
    return vertices;
}

bool AABBBV::isOnOrForwardPlane(const Plane& plane) const
{
    const float r = extents.x * std::abs(plane.normal.x) + extents.y * std::abs(plane.normal.y) +
                    extents.z * std::abs(plane.normal.z);

    return -r <= plane.getSignedDistanceToPlane(center);
}

bool AABBBV::isOnFrustum(const Frustum& camFrustum, const Transform& transform) const
{
    const glm::vec3 globalCenter{ transform.globalMatrix * glm::vec4(center, 1.f) };
    const glm::vec3 right = transform.globalMatrix[0] * extents.x;
    const glm::vec3 up = transform.globalMatrix[1] * extents.y;
    const glm::vec3 forward = -transform.globalMatrix[2] * extents.z;

    const float newIi = std::abs(glm::dot(glm::vec3{ 1.f, 0.f, 0.f }, right)) +
                        std::abs(glm::dot(glm::vec3{ 1.f, 0.f, 0.f }, up)) +
                        std::abs(glm::dot(glm::vec3{ 1.f, 0.f, 0.f }, forward));

    const float newIj = std::abs(glm::dot(glm::vec3{ 0.f, 1.f, 0.f }, right)) +
                        std::abs(glm::dot(glm::vec3{ 0.f, 1.f, 0.f }, up)) +
                        std::abs(glm::dot(glm::vec3{ 0.f, 1.f, 0.f }, forward));

    const float newIk = std::abs(glm::dot(glm::vec3{ 0.f, 0.f, 1.f }, right)) +
                        std::abs(glm::dot(glm::vec3{ 0.f, 0.f, 1.f }, up)) +
                        std::abs(glm::dot(glm::vec3{ 0.f, 0.f, 1.f }, forward));

    const AABBBV globalAABB(globalCenter, newIi, newIj, newIk);

    return (globalAABB.isOnOrForwardPlane(camFrustum.leftFace) &&
            globalAABB.isOnOrForwardPlane(camFrustum.rightFace) &&
            globalAABB.isOnOrForwardPlane(camFrustum.topFace) &&
            globalAABB.isOnOrForwardPlane(camFrustum.bottomFace) &&
            globalAABB.isOnOrForwardPlane(camFrustum.nearFace) &&
            globalAABB.isOnOrForwardPlane(camFrustum.farFace));
}

nlohmann::json AABBBV::serialize() const
{
	nlohmann::json j;
	j["type"] = "AABB";
	j["center"] = center;
	j["extents"] = extents;
	return j;
}