//
// Created by lukas on 27.04.2025.
//

#ifndef BOUNDINGVOLUMES_H
#define BOUNDINGVOLUMES_H

#include "glm/glm.hpp"
#include "Frustum.h"



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

	static BoundingBox calculateBoundingBox(const std::vector<Mesh>& meshes);

    std::array<glm::vec3, 8> getVertices() const;
    glm::vec3 getCenter() const { return center;}
    glm::vec3 getExtents() const { return extents;}
    BoundingBox merge(BoundingBox other);

    BoundingBox getGlobalBox(const Transform& transform) const;
    glm::vec3 getGlobalCenter(const Transform& transform) const;
	nlohmann::json serialize() const;

    glm::vec3 center;
    glm::vec3 extents;
};


#endif //BOUNDINGVOLUMES_H
