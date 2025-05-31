#include "CollisionSystem.h"

#include "Scene.h"
#include "Components.h"

#include <functional>

struct ColliderObjectInfo
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

using CollisionFunction = std::function<CollisionEvent(const ColliderObjectInfo&, const ColliderObjectInfo&)>;

static CollisionEvent BoxBoxCollision(const ColliderObjectInfo& boxA, const ColliderObjectInfo& boxB);
static CollisionEvent BoxSphereCollision(const ColliderObjectInfo& box, const ColliderObjectInfo& sphere);
static CollisionEvent SphereSphereCollision(const ColliderObjectInfo& sphereA, const ColliderObjectInfo& sphereB);

static std::unordered_map<
	std::pair<ColliderType, ColliderType>,
	CollisionFunction,
	ColliderTypePairHash> collisionFunctions = 
	{
		{{ColliderType::BOX, ColliderType::BOX}, BoxBoxCollision},
		{{ColliderType::BOX, ColliderType::SPHERE}, BoxSphereCollision},
		{{ColliderType::SPHERE, ColliderType::SPHERE}, SphereSphereCollision},
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

	auto colliderStorage = scene->getStorage<ColliderComponent>();
    if(colliderStorage == nullptr) {
        return;
    }
	auto transformStorage = scene->getStorage<Transform>();

	std::vector<ColliderObjectInfo> colliderObjects;

	for (int i = 0; i < colliderStorage->getQuantity(); i++)
	{
		auto& colliderComponent = colliderStorage->components[i];
		colliderObjects.push_back({
				.transform = &transformStorage->get(colliderComponent.id),
				.collider = &colliderComponent,
				.shape = colliderComponent.GetColliderShape()
			});
	}

	for (size_t i = 0; i < colliderObjects.size(); ++i)
	{
		for (size_t j = i + 1; j < colliderObjects.size(); ++j)
		{
			ColliderObjectInfo objectFirst = colliderObjects[i];
			ColliderObjectInfo objectSecond = colliderObjects[j];

			if (objectFirst.collider->isStatic && objectSecond.collider->isStatic)
				continue;

			CollisionEvent collisionInfo{};

			if (objectFirst.shape->getType() > objectSecond.shape->getType())
				std::swap(objectFirst, objectSecond);

			std::pair<ColliderType, ColliderType> shapeTypePair = {
				objectFirst.shape->getType(),
				objectSecond.shape->getType()
			};

			auto it = collisionFunctions.find(shapeTypePair);
			if (it != collisionFunctions.end())
			{
				CollisionFunction collisionFunction = it->second;
				collisionInfo = collisionFunction(objectFirst, objectSecond);
			}

			if (collisionInfo.isColliding)
			{
				collisionInfo.objectA = objectFirst.collider->id;
				collisionInfo.objectB = objectSecond.collider->id;

				collisions.push_back(collisionInfo);
				scene->getEventSystem().queueEvent(collisionInfo);

				CollisionEvent swappedCollisionInfo = collisionInfo;
				swappedCollisionInfo.objectA = objectSecond.collider->id;
				swappedCollisionInfo.objectB = objectFirst.collider->id;
				swappedCollisionInfo.separationVector = -swappedCollisionInfo.separationVector;

				scene->getEventSystem().queueEvent(swappedCollisionInfo);
			}

		}
	}
}


// helper functions

struct OBB
{
	glm::vec3 center;
	glm::vec3 halfSize;
	glm::vec3 axes[3];
};

static OBB boxColliderToOBB(BoxCollider* shape, Transform* transform)
{
	OBB obb;
	const glm::mat4& modelMatrix = transform->globalMatrix;

	obb.center = glm::vec3(modelMatrix * glm::vec4(shape->center, 1.0f));

	obb.halfSize = shape->halfSize;
	obb.halfSize.x *= glm::length(glm::vec3(modelMatrix[0]));
	obb.halfSize.y *= glm::length(glm::vec3(modelMatrix[1]));
	obb.halfSize.z *= glm::length(glm::vec3(modelMatrix[2]));

	for (int i = 0; i < 3; ++i)
	{
		glm::vec3 axis = glm::vec3(modelMatrix[i]);
		float length = glm::length(axis);
		if (length > 0.0f)
		{
			obb.axes[i] = axis / length;
		}
		else
		{
			obb.axes[i] = glm::vec3(0.0f);
		}
	}


	return obb;
}

static float sphereColliderWorldRadius(SphereCollider* shape, Transform* transform)
{
	const glm::mat4& modelMatrix = transform->globalMatrix;
	glm::vec3 scale;
	scale.x = glm::length(glm::vec3(modelMatrix[0]));
	scale.y = glm::length(glm::vec3(modelMatrix[1]));
	scale.z = glm::length(glm::vec3(modelMatrix[2]));
	float maxScale = std::max(scale.x, std::max(scale.y, scale.z));

	return shape->radius * maxScale;
}

static float SATgetAxisOverlap(const glm::vec3& axis, const OBB& obbA, const OBB& obbB)
{
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

static glm::vec3 closestPointOnOBB(const OBB& obb, const glm::vec3& point)
{
	glm::vec3 closestPoint = obb.center;

	for (int i = 0; i < 3; ++i)
	{
		float distance = glm::dot(point - obb.center, obb.axes[i]);
		distance = glm::clamp(distance, -obb.halfSize[i], obb.halfSize[i]);
		closestPoint += distance * obb.axes[i];
	}

	return closestPoint;
}


// collision functions


static CollisionEvent BoxBoxCollision(const ColliderObjectInfo& boxA, const ColliderObjectInfo& boxB)
{
	CollisionEvent collisionInfo = {};

	OBB obbA = boxColliderToOBB(static_cast<BoxCollider*>(boxA.shape), boxA.transform);
	OBB obbB = boxColliderToOBB(static_cast<BoxCollider*>(boxB.shape), boxB.transform);

	glm::vec3 axes[15];

	for (int i = 0; i < 3; ++i)
	{
		axes[i] = obbA.axes[i];
		axes[i + 3] = obbB.axes[i];
		for (int j = 0; j < 3; ++j)
		{
			axes[3 * i + j + 6] = glm::cross(obbA.axes[i], obbB.axes[j]);
		}
	}

	float minOverlap = std::numeric_limits<float>::max();
	glm::vec3 minOverlapAxis;

	for (int i = 0; i < 15; ++i)
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


static CollisionEvent BoxSphereCollision(const ColliderObjectInfo& box, const ColliderObjectInfo& sphere)
{
	CollisionEvent collisionInfo = {};

	OBB obb = boxColliderToOBB(static_cast<BoxCollider*>(box.shape), box.transform);

	SphereCollider* sphereCollider = static_cast<SphereCollider*>(sphere.shape);
	glm::vec3 sphereCenter = sphere.transform->globalMatrix * glm::vec4(sphereCollider->center, 1.0f);
	float sphereRadius = sphereColliderWorldRadius(sphereCollider, sphere.transform);

	glm::vec3 delta = sphereCenter - closestPointOnOBB(obb, sphereCenter);
	float distance = glm::length(delta);

	if (distance < sphereRadius)
	{
		collisionInfo.isColliding = true;
		float separationDistance = distance - sphereRadius;

		if (distance > 0.0f)
		{
			collisionInfo.separationVector = delta / distance * (distance - sphereRadius);
		}
		else if (sphereCenter == obb.center)
		{
			collisionInfo.separationVector = glm::vec3(0.0f);
		}
		else
		{
			glm::vec3 local = glm::vec3(
				glm::dot(delta, obb.axes[0]),
				glm::dot(delta, obb.axes[1]),
				glm::dot(delta, obb.axes[2])
			);

			int maxAxis = 0;
			float maxVal = std::abs(local[0]);
			for (int i = 1; i < 3; ++i)
			{
				float absVal = std::abs(local[i]);
				if (absVal > maxVal)
				{
					maxAxis = i;
					maxVal = absVal;
				}
			}

			collisionInfo.separationVector = obb.axes[maxAxis] * (local[maxAxis] < 0.0f ? 1.0f : -1.0f) * separationDistance;
		}
	}

	return collisionInfo;
}

static CollisionEvent SphereSphereCollision(const ColliderObjectInfo& sphereA, const ColliderObjectInfo& sphereB)
{
	CollisionEvent collisionInfo = {};

	SphereCollider* sphereColliderA = static_cast<SphereCollider*>(sphereA.shape);
	SphereCollider* sphereColliderB = static_cast<SphereCollider*>(sphereB.shape);

	glm::vec3 centerA = sphereA.transform->globalMatrix * glm::vec4(sphereColliderA->center, 1.0f);
	glm::vec3 centerB = sphereB.transform->globalMatrix * glm::vec4(sphereColliderB->center, 1.0f);

	float radiusA = sphereColliderWorldRadius(sphereColliderA, sphereA.transform);
	float radiusB = sphereColliderWorldRadius(sphereColliderB, sphereB.transform);

	glm::vec3 delta = centerA - centerB;
	float distance = glm::length(delta);
	float radiusSum = radiusA + radiusB;

	if (distance < radiusSum)
	{
		collisionInfo.isColliding = true;
		if (distance > 0.0f)
		{
			collisionInfo.separationVector = delta / distance * (radiusSum - distance);
		}
		else
		{
			collisionInfo.separationVector = glm::vec3(0.0f);
		}
	}

	return collisionInfo;
}