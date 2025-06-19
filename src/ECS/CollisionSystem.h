#pragma once

#include <glm/vec3.hpp>
#include <vector>
#include "EntityManager.h"
#include "EventSystem.h"
#include "KDTree.h"

class Scene;

struct OBB
{
	glm::vec3 center;
	glm::vec3 halfSize;
	glm::vec3 axes[3];
};

struct GlobalSphere
{
	glm::vec3 center;
	float radius;
};

struct ColliderObjectInfo
{
	ColliderObjectInfo(ColliderComponent* collider, Transform* transform);

	Transform* transform;
	ColliderComponent* collider;
	ColliderShape* shape;
	union
	{
		OBB obb;
		GlobalSphere sphere;
	} globalShape;
	BoundingBox boundingBox;
};


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

	BVH<ColliderObjectInfo> staticColliderTree;

	void checkPair(const ColliderObjectInfo& objectFirst, const ColliderObjectInfo& objectSecond);

public:
	CollisionSystem(Scene* scene);
	~CollisionSystem();

	void buildTree();

	void CheckCollisions();
	std::vector<CollisionEvent> const& GetCollisions() const
	{
		return collisions;
	}
};