#pragma once

#include <glm/vec3.hpp>
#include <vector>
#include "EntityManager.h"

class Scene;


struct CollisionInfo
{
	bool isColliding = false;
	EntityID objectA = -1;
	EntityID objectB = -1;
	glm::vec3 separationVector;
};

class CollisionSystem
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