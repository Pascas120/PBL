//
// Created by lukas on 27.04.2025.
//

#ifndef BOUNDINGVOLUMES_H
#define BOUNDINGVOLUMES_H

#include "Export.h"

#include "glm/glm.hpp"
#include "Camera.h"
#include "Components.h"

struct Transform;

class ENGINE_API BoundingVolume
{
public:
    virtual ~BoundingVolume() = default;

    virtual bool isOnFrustum(const Frustum& camFrustum, const Transform& transform) const = 0;
    virtual bool isOnOrForwardPlane(const Plane& plane) const = 0;

    bool isOnFrustum(const Frustum& camFrustum) const;
};

class ENGINE_API SphereBV : public BoundingVolume
{
public:
    SphereBV(const glm::vec3& inCenter, float inRadius);

    bool isOnOrForwardPlane(const Plane& plane) const override;
    bool isOnFrustum(const Frustum& camFrustum, const Transform& transform) const override;

private:
    glm::vec3 center;
    float radius;
};

class ENGINE_API AABBBV : public BoundingVolume
{
public:
    AABBBV(const glm::vec3& min, const glm::vec3& max);
    AABBBV(const glm::vec3& inCenter, float iI, float iJ, float iK);

    std::array<glm::vec3, 8> getVertices() const;

    bool isOnOrForwardPlane(const Plane& plane) const override;
    bool isOnFrustum(const Frustum& camFrustum, const Transform& transform) const override;

private:
    glm::vec3 center;
    glm::vec3 extents;
};


#endif //BOUNDINGVOLUMES_H
