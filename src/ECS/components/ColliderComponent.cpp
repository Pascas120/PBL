#include "ColliderComponent.h"
#include <memory>

ColliderComponent::ColliderComponent(ColliderType colliderType, bool isStatic) : isStatic{ isStatic }
{
    switch (colliderType)
    {
    case ColliderType::BOX:
        colliderShape = std::make_unique<BoxCollider>();
        break;
    case ColliderType::SPHERE:
        colliderShape = std::make_unique<SphereCollider>();
        break;
    default:
        colliderShape = nullptr;
        break;
    }
}
