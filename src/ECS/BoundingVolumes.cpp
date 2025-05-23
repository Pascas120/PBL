#include "BoundingVolumes.h"
#include "Camera.h"
#include "Components.h"

// temporary
#include "glm_serialization.h"


// Sphere implementation
SphereBV::SphereBV(const glm::vec3& inCenter, float inRadius)
    : center(inCenter), radius(inRadius) {}


SphereBV SphereBV::calculateBoundingSphere(const std::vector<Mesh>& meshes)
{
	std::vector<Vertex> vertices;
	for (const auto& mesh : meshes)
	{
		vertices.insert(vertices.end(), mesh.vertices.begin(), mesh.vertices.end());
	}
    int index1 = -1, index2 = -1;
	float maxDistance2 = 0.0f;

	for (int i = 0; i < vertices.size(); ++i)
	{
		for (int j = i + 1; j < vertices.size(); ++j)
		{
			float distance2 = glm::distance2(vertices[i].Position, vertices[j].Position);
			if (distance2 > maxDistance2)
			{
				maxDistance2 = distance2;
				index1 = i;
				index2 = j;
			}
		}
	}
	glm::vec3 center = (vertices[index1].Position + vertices[index2].Position) * 0.5f;
	float sqRadius = maxDistance2 * 0.25f;
	float radius = std::sqrt(sqRadius);

	for (auto& vertex : vertices)
	{
        glm::vec3 point = vertex.Position;

		glm::vec3 offset = point - center;
		float distance2 = glm::dot(offset, offset);

		if (distance2 > sqRadius)
		{
			float distance = std::sqrt(distance2);
			float radiusDiff = (distance - radius) / 2.0f;

			glm::vec3 newCenter = center + offset * radiusDiff;

			radius += radiusDiff;
			sqRadius = radius * radius;
		}
	}

	return SphereBV(center, radius);
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

AABBBV AABBBV::calculateBoundingBox(const std::vector<Mesh>& meshes)
{
    glm::vec3 min = glm::vec3(FLT_MAX);
    glm::vec3 max = glm::vec3(-FLT_MAX);

    for (const auto& mesh : meshes)
    {
        for (const auto& vertex : mesh.vertices)
        {
            min = glm::min(min, vertex.Position);
            max = glm::max(max, vertex.Position);
        }
    }

    return AABBBV(min, max);
}

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

nlohmann::json AABBBV::serialize() const
{
	nlohmann::json j;
	j["type"] = "AABB";
	j["center"] = center;
	j["extents"] = extents;
	return j;
}