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

class BoundingVolume
{
public:
    virtual ~BoundingVolume() = default;

	virtual nlohmann::json serialize() const = 0;
};

class SphereBV : public BoundingVolume
{
public:
    SphereBV() = default;
    SphereBV(const glm::vec3& inCenter, float inRadius);

    static SphereBV calculateBoundingSphere(const std::vector<Mesh>& meshes);

    nlohmann::json serialize() const override;

    glm::vec3 center;
    float radius;
};

class AABBBV : public BoundingVolume
{
public:
    AABBBV() = default;
    AABBBV(const glm::vec3& min, const glm::vec3& max);
    AABBBV(const glm::vec3& inCenter, float iI, float iJ, float iK);

	static AABBBV calculateBoundingBox(const std::vector<Mesh>& meshes);

    std::array<glm::vec3, 8> getVertices() const;

	nlohmann::json serialize() const override;

    glm::vec3 center;
    glm::vec3 extents;
};


#endif //BOUNDINGVOLUMES_H
