#pragma once

#include <glm/vec3.hpp>
#include <vector>
#include "EntityManager.h"
#include "EventSystem.h"

class Scene;


struct CollisionEvent final : public Event
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
	std::vector<CollisionEvent> collisions;

public:
	CollisionSystem(Scene* scene);
	~CollisionSystem();

	void CheckCollisions();
	std::vector<CollisionEvent> const& GetCollisions() const
	{
		return collisions;
	}
};