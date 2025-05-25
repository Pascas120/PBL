//
// Created by lukas on 27.04.2025.
//

#ifndef BOUNDINGVOLUMES_H
#define BOUNDINGVOLUMES_H

#include "glm/glm.hpp"
//#include "Camera.h"
//#include "Components.h"

class Frustum;
class Plane;
class Transform;
class Mesh;

// temporary
#include "nlohmann/json.hpp"

struct Transform;

class BoundingBox
{
public:
    BoundingBox(const glm::vec3& min, const glm::vec3& max);
    BoundingBox(const glm::vec3& inCenter, float iI, float iJ, float iK);
    BoundingBox();

	static AABBBV calculateBoundingBox(const std::vector<Mesh>& meshes);

    std::array<glm::vec3, 8> getVertices() const;
    glm::vec3 getCenter() const { return center;}
    glm::vec3 getExtents() const { return extents;}
    BoundingBox merge(BoundingBox other);

    bool isOnOrForwardPlane(const Plane& plane) const;
    bool isOnFrustum(const Frustum& camFrustum, const Transform& transform) const;
    BoundingBox getGlobalBox(const Transform& transform) const;
    glm::vec3 getGlobalCenter(const Transform& transform) const;
	nlohmann::json serialize() const override;

    glm::vec3 center;
    glm::vec3 extents;
};


#endif //BOUNDINGVOLUMES_H
