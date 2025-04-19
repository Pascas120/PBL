#include "ColliderComponent.h"

ColliderComponent::ColliderComponent(ColliderType colliderType, bool isStatic)
	: isStatic(isStatic)
{
	switch (colliderType)
	{
	case ColliderType::BOX:
		colliderShape = new BoxCollider();
		break;
	case ColliderType::SPHERE:
		colliderShape = new SphereCollider();
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