#include "Random.h"

#include <random>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/type_ptr.hpp>

static std::mt19937 rng(std::random_device{}());

int Random::getInt(int minInclusive, int maxExclusive) {
	std::uniform_int_distribution<int> dist(minInclusive, maxExclusive - 1);
	return dist(rng);
}

float Random::getFloat(float minInclusive, float maxExclusive) {
	std::uniform_real_distribution<float> dist(minInclusive, maxExclusive);
	return dist(rng);
}

glm::vec2 Random::inUnitCircle() {
	float angle = getFloat(0.0f, glm::two_pi<float>());
	float radius = getFloat(0.0f, 1.0f);
	return glm::vec2(radius * cos(angle), radius * sin(angle));
}

glm::vec3 Random::inUnitSphere()
{
    while (true)
    {
        float x = getFloat(-1.0f, 1.0f);
        float y = getFloat(-1.0f, 1.0f);
        float z = getFloat(-1.0f, 1.0f);
        glm::vec3 v(x, y, z);
		if (glm::length(v) <= 1.0f) {
			return v;
		}
    }
}
