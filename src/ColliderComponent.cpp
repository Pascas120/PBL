#include "ColliderComponent.h"

ColliderComponent::ColliderComponent(ColliderType colliderType)
{
	switch (colliderType)
	{
	case ColliderType::BOX:
		colliderShape = new BoxCollider();
		break;
	case ColliderType::SPHERE:
		colliderShape = new SphereCollider();
		break;
	case ColliderType::CAPSULE:
		colliderShape = new CapsuleCollider();
		break;
	default:
		colliderShape = nullptr;
		break;
	}
}

ColliderComponent::~ColliderComponent()
{
	if (colliderShape)
	{
		delete colliderShape;
	}
}