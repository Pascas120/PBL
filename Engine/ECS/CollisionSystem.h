#pragma once

#include "Export.h"

#include <glm/vec3.hpp>
#include <vector>
#include "EntityManager.h"

class Scene;


struct ENGINE_API CollisionInfo
{
	bool isColliding = false;
	EntityID objectA = -1;
	EntityID objectB = -1;
	glm::vec3 separationVector;
};

class ENGINE_API CollisionSystem
{
private:
	Scene* scene;
	std::vector<CollisionInfo> collisions;

public:
	CollisionSystem(Scene* scene);
	~CollisionSystem();

	void CheckCollisions();
	std::vector<CollisionInfo> const& GetCollisions() const
	{
		return collisions;
	}
};