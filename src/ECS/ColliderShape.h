#pragma once

#include "glm/vec3.hpp"

enum class ColliderType
{
	BOX = 0,
	SPHERE,
};

struct ColliderShape
{
	glm::vec3 center;

	virtual ColliderType getType() const = 0;
};

struct BoxCollider : public ColliderShape
{
	glm::vec3 halfSize = glm::vec3(1.0f, 1.0f, 1.0f);

	ColliderType getType() const override
	{
		return ColliderType::BOX;
	}
};

struct SphereCollider : public ColliderShape
{
	float radius = 0.5f;

	ColliderType getType() const override
	{
		return ColliderType::SPHERE;
	}
};