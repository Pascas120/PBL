#include "BoundingVolumes.h"


// bool BoundingBox::isOnFrustum(const Frustum& camFrustum) const
// {
//     return (isOnOrForwardPlane(camFrustum.leftFace) &&
//             isOnOrForwardPlane(camFrustum.rightFace) &&
//             isOnOrForwardPlane(camFrustum.topFace) &&
//             isOnOrForwardPlane(camFrustum.bottomFace) &&
//             isOnOrForwardPlane(camFrustum.nearFace) &&
//             isOnOrForwardPlane(camFrustum.farFace));
// }

BoundingBox::BoundingBox()
    : center(0.f), extents(0.f) {}

BoundingBox::BoundingBox(const glm::vec3& min, const glm::vec3& max)
    : center((max + min) * 0.5f), extents(max.x - center.x, max.y - center.y, max.z - center.z) {}

BoundingBox::BoundingBox(const glm::vec3& inCenter, float iI, float iJ, float iK)
    : center(inCenter), extents(iI, iJ, iK) {}

std::array<glm::vec3, 8> BoundingBox::getVertices() const
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

BoundingBox BoundingBox::merge(BoundingBox other) {
    glm::vec3 newMin = glm::min(center - extents, other.center - other.extents);
    glm::vec3 newMax = glm::max(center + extents, other.center + other.extents);
    return BoundingBox(newMin, newMax);
}

bool BoundingBox::isOnOrForwardPlane(const Plane& plane) const
{
    const float r = extents.x * std::abs(plane.normal.x) + extents.y * std::abs(plane.normal.y) +
                    extents.z * std::abs(plane.normal.z);

    return -r <= plane.getSignedDistanceToPlane(center);
}

bool BoundingBox::isOnFrustum(const Frustum& camFrustum, const Transform& transform) const
{

    const BoundingBox globalAABB = getGlobalBox(transform);

    return (globalAABB.isOnOrForwardPlane(camFrustum.leftFace) &&
            globalAABB.isOnOrForwardPlane(camFrustum.rightFace) &&
            globalAABB.isOnOrForwardPlane(camFrustum.topFace) &&
            globalAABB.isOnOrForwardPlane(camFrustum.bottomFace) &&
            globalAABB.isOnOrForwardPlane(camFrustum.nearFace) &&
            globalAABB.isOnOrForwardPlane(camFrustum.farFace));
}

BoundingBox BoundingBox::getGlobalBox(const Transform &transform) const {
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

    return BoundingBox (globalCenter, newIi, newIj, newIk);
}

glm::vec3 BoundingBox::getGlobalCenter(const Transform &transform) const {
    return transform.globalMatrix * glm::vec4(center, 1.f);
}
