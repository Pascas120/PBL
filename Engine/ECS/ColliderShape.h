#pragma once

#include "Export.h"

#include "glm/vec3.hpp"

enum class ENGINE_API ColliderType
{
	BOX = 0,
	SPHERE,
};

struct ENGINE_API ColliderShape
{
	glm::vec3 center;

	virtual ColliderType getType() const = 0;
	virtual ~ColliderShape() = default;
};

struct ENGINE_API BoxCollider : public ColliderShape
{
	glm::vec3 halfSize = glm::vec3(1.0f, 1.0f, 1.0f);

	ColliderType getType() const override
	{
		return ColliderType::BOX;
	}
};

struct ENGINE_API SphereCollider : public ColliderShape
{
	float radius = 0.5f;

	ColliderType getType() const override
	{
		return ColliderType::SPHERE;
	}
};