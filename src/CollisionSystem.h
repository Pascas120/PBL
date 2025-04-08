#pragma once

#include "glm/vec3.hpp"

class Scene;
class GameObject;


//poddaje sie
#include <vector>

struct CollisionInfo
{
	bool isColliding = false;
	GameObject* objectA = nullptr;
	GameObject* objectB = nullptr;
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
	std::vector<CollisionInfo> const &GetCollisions() const
	{
		return collisions;
	}
};