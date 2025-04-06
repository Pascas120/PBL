#include "CollisionSystem.h"

#include "Scene.h"
#include "Transform.h"
#include "ColliderComponent.h"
#include <stack>
#include <unordered_map>
#include <functional>
#include <spdlog/spdlog.h>

struct TransformCollider
{
	Transform* transform;
	ColliderComponent* collider;
	ColliderShape* shape;
};

struct ColliderTypePairHash
{
	size_t operator()(const std::pair<ColliderType, ColliderType>& pair) const
	{
		return std::hash<int>()(static_cast<int>(pair.first)) ^ std::hash<int>()(static_cast<int>(pair.second));
	}
};

using CollisionFunction = std::function<CollisionInfo(const TransformCollider, const TransformCollider)>;

static CollisionInfo BoxBoxCollision(const TransformCollider objA, const TransformCollider objB);
static CollisionInfo BoxSphereCollision(const TransformCollider objA, const TransformCollider objB);
static CollisionInfo BoxCapsuleCollision(const TransformCollider objA, const TransformCollider objB);
static CollisionInfo SphereSphereCollision(const TransformCollider objA, const TransformCollider objB);
static CollisionInfo SphereCapsuleCollision(const TransformCollider objA, const TransformCollider objB);
static CollisionInfo CapsuleCapsuleCollision(const TransformCollider objA, const TransformCollider objB);

static std::unordered_map<
	std::pair<ColliderType, ColliderType>,
	CollisionFunction, 
	ColliderTypePairHash> collisionFunctions = 
	{
		{{ColliderType::BOX, ColliderType::BOX}, BoxBoxCollision},
		{{ColliderType::BOX, ColliderType::SPHERE}, BoxSphereCollision},
		{{ColliderType::BOX, ColliderType::CAPSULE}, BoxCapsuleCollision},
		{{ColliderType::SPHERE, ColliderType::SPHERE}, SphereSphereCollision},
		{{ColliderType::SPHERE, ColliderType::CAPSULE}, SphereCapsuleCollision},
		{{ColliderType::CAPSULE, ColliderType::CAPSULE}, CapsuleCapsuleCollision}
	};



CollisionSystem::CollisionSystem(Scene* scene) : scene(scene)
{
}

CollisionSystem::~CollisionSystem()
{
}


void CollisionSystem::CheckCollisions()
{
	collisions.clear();
	std::vector<GameObject*> objects;
	std::stack<GameObject*> stack;
	stack.push(scene->GetRoot());

	while (!stack.empty())
	{
		GameObject* current = stack.top();
		stack.pop();

		if (current->components.GetComponent<ColliderComponent>() != nullptr)
		{
			objects.push_back(current);
		}

		GameObject** children = current->GetChildren();
		for (int i = 0; i < current->GetChildCount(); ++i)
		{
			stack.push(children[i]);
		}
	}

	for (size_t i = 0; i < objects.size(); ++i)
	{
		for (size_t j = i + 1; j < objects.size(); ++j)
		{
			Transform* transformFirst = objects[i]->components.GetComponent<Transform>();
			Transform* transformSecond = objects[j]->components.GetComponent<Transform>();
			ColliderComponent* colliderFirst = objects[i]->components.GetComponent<ColliderComponent>();
			ColliderComponent* colliderSecond = objects[j]->components.GetComponent<ColliderComponent>();

			if (transformFirst && transformSecond && colliderFirst && colliderSecond)
			{
				CollisionInfo collisionInfo = {};

				ColliderShape* shapeFirst = colliderFirst->GetColliderShape();
				ColliderShape* shapeSecond = colliderSecond->GetColliderShape();

				bool swapOrder = shapeFirst->getType() > shapeSecond->getType();

				if (swapOrder)
				{
					std::swap(transformFirst, transformSecond);
					std::swap(colliderFirst, colliderSecond);
					std::swap(shapeFirst, shapeSecond);
				}
				std::pair<ColliderType, ColliderType> colliderPair = { shapeFirst->getType(), shapeSecond->getType() };
				auto it = collisionFunctions.find(colliderPair);
				if (it != collisionFunctions.end())
				{
					CollisionFunction collisionFunction = it->second;
					collisionInfo = collisionFunction(
						{ transformFirst, colliderFirst, shapeFirst },
						{ transformSecond, colliderSecond, shapeSecond }
					);
				}

				if (collisionInfo.isColliding)
				{
					collisionInfo.objectA = objects[i];
					collisionInfo.objectB = objects[j];

					collisions.push_back(collisionInfo);
				}
			}
		}
	}

}

//static CollisionInfo BoxBoxCollision(const BoxCollider& boxA, const BoxCollider& boxB)
//{
//	CollisionInfo collisionInfo = {};
//
//	return collisionInfo;
//}
static CollisionInfo BoxBoxCollision(const TransformCollider objA, const TransformCollider objB)
{
	CollisionInfo collisionInfo = {};

	spdlog::info("Checking Box-Box Collision");

	return collisionInfo;
}

static CollisionInfo BoxSphereCollision(const TransformCollider objA, const TransformCollider objB)
{
	CollisionInfo collisionInfo = {};

	spdlog::info("Checking Box-Sphere Collision");

	return collisionInfo;
}

static CollisionInfo BoxCapsuleCollision(const TransformCollider objA, const TransformCollider objB)
{
	CollisionInfo collisionInfo = {};

	spdlog::info("Checking Box-Capsule Collision");

	return collisionInfo;
}

static CollisionInfo SphereSphereCollision(const TransformCollider objA, const TransformCollider objB)
{
	CollisionInfo collisionInfo = {};

	spdlog::info("Checking Sphere-Sphere Collision");

	return collisionInfo;
}

static CollisionInfo SphereCapsuleCollision(const TransformCollider objA, const TransformCollider objB)
{
	CollisionInfo collisionInfo = {};

	spdlog::info("Checking Sphere-Capsule Collision");

	return collisionInfo;
}

static CollisionInfo CapsuleCapsuleCollision(const TransformCollider objA, const TransformCollider objB)
{
	CollisionInfo collisionInfo = {};

	spdlog::info("Checking Capsule-Capsule Collision");

	return collisionInfo;
}

