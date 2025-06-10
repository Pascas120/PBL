#pragma once

#include <glm/glm.hpp>

namespace Random 
{
	int getInt(int minInclusive, int maxExclusive);
	float getFloat(float minInclusive, float maxExclusive);

	glm::vec2 inUnitCircle();
	glm::vec3 inUnitSphere();
}