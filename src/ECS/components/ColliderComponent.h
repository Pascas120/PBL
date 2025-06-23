#pragma once

#include "ECS/ColliderShape.h"
#include "ECS/EntityManager.h"

struct ColliderPropertyFlags {
    enum {
        None = 0,
        DisableButterSticking = 1 << 0,
        DisableButterTrail = 1 << 1,
        DisableCollider = 1 << 2,
    };
};

struct ColliderComponent {
    ColliderComponent(ColliderType colliderType, bool isStatic = true);
    ColliderComponent() = default;

    // unsafe (TODO)
    ColliderShape* GetColliderShape() const
    {
        return colliderShape.get();
    }

    bool isStatic = true;
    bool isTrigger = false;

    int properties = ColliderPropertyFlags::None;


    EntityID id = (EntityID)-1;

private:
    std::shared_ptr<ColliderShape> colliderShape = nullptr;
};