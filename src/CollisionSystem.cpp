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
					spdlog::info("Checking collision between {} and {}", objects[i]->GetName(), objects[j]->GetName());
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
					if (swapOrder)
					{
						std::swap(collisionInfo.objectA, collisionInfo.objectB);
					}

					collisions.push_back(collisionInfo);
				}
			}
		}
	}

}


// TODO: move to another file
struct OBB
{
	glm::vec3 center;
	glm::vec3 halfSize;
	glm::vec3 axes[3];
};

static OBB boxColliderToOBB(BoxCollider* shape, Transform* transform)
{
	OBB obb;
	const glm::mat4& modelMatrix = transform->getModelMatrix();

	obb.center = glm::vec3(modelMatrix * glm::vec4(shape->center, 1.0f));

	obb.halfSize = shape->halfSize;
	obb.halfSize.x *= glm::length(glm::vec3(modelMatrix[0]));
	obb.halfSize.y *= glm::length(glm::vec3(modelMatrix[1]));
	obb.halfSize.z *= glm::length(glm::vec3(modelMatrix[2]));

	obb.axes[0] = glm::normalize(glm::vec3(modelMatrix[0]));
	obb.axes[1] = glm::normalize(glm::vec3(modelMatrix[1]));
	obb.axes[2] = glm::normalize(glm::vec3(modelMatrix[2]));


	return obb;
}

static float SATgetAxisOverlap(const glm::vec3& axis, const OBB& obbA, const OBB& obbB)
{
	/*float p0 = glm::dot(axis, obbA.center - obbB.center);
	float r0 = glm::dot(axis, obbA.halfSize);
	float r1 = glm::dot(axis, obbB.halfSize);

	
	return r0 + r1 - std::abs(p0);*/

	float projA = 0.0f;
	float projB = 0.0f;

	for (int i = 0; i < 3; ++i)
	{
		projA += obbA.halfSize[i] * std::abs(glm::dot(axis, obbA.axes[i]));
		projB += obbB.halfSize[i] * std::abs(glm::dot(axis, obbB.axes[i]));
	}

	float distance = std::abs(glm::dot(axis, obbA.center - obbB.center));
	return projA + projB - distance;
}	

static CollisionInfo BoxBoxCollision(const TransformCollider objA, const TransformCollider objB)
{
	CollisionInfo collisionInfo = {};
	spdlog::info("Checking Box-Box Collision");

	OBB obbA = boxColliderToOBB(static_cast<BoxCollider*>(objA.shape), objA.transform);
	OBB obbB = boxColliderToOBB(static_cast<BoxCollider*>(objB.shape), objB.transform);

	glm::vec3 axes[15];
	int axes_i = 0;

	for (int i = 0; i < 3; ++i)
	{
		axes[axes_i++] = obbA.axes[i];
	}
	for (int i = 0; i < 3; ++i)
	{
		axes[axes_i++] = obbB.axes[i];
	}
	for (int i = 0; i < 3; ++i)
	{
		for (int j = 0; j < 3; ++j)
		{
			axes[axes_i++] = glm::cross(obbA.axes[i], obbB.axes[j]);
		}
	}

	float minOverlap = std::numeric_limits<float>::max();
	glm::vec3 minOverlapAxis;

	for (int i = 0; i < axes_i; ++i)
	{
		glm::vec3 normalizedAxis = glm::normalize(axes[i]);
		float overlap = SATgetAxisOverlap(normalizedAxis, obbA, obbB);
		if (overlap <= 0.0f)
		{
			return collisionInfo;
		}
		
		if (overlap < minOverlap)
		{
			minOverlap = overlap;
			minOverlapAxis = normalizedAxis;
			if (glm::dot(minOverlapAxis, obbA.center - obbB.center) < 0.0f)
			{
				minOverlapAxis = -minOverlapAxis;
			}
		}
	}

	collisionInfo.isColliding = true;
	collisionInfo.separationVector = minOverlapAxis * minOverlap;



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

