//
// Created by Łukasz Moskwin on 14/04/2025.
//

#ifndef PBL_COMPONENTS_H
#define PBL_COMPONENTS_H
#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include <string>
#include "Model.h"
#include "EntityManager.h"
#include "ColliderShape.h"

struct Transform {
    glm::vec3 translation = {0.0f, 0.0f, 0.0f};
    glm::quat rotation = {1.0f, 0.0f, 0.0f, 0.0f};
    glm::vec3 scale = {1.0f, 1.0f, 1.0f};
    glm::mat4 globalMatrix = glm::mat4(1.0f);

    bool isDirty = false;
    std::vector<EntityID> children;
    EntityID parent = (EntityID) - 1;
};

struct ModelComponent {
    Model* model;
};

struct ImageComponent {
    std::string texturePath;
    float width;
    float height;
    glm::vec4 color;
};

struct TextComponent {
    std::string font;
    glm::vec4 color;
    std::string text;
};

struct ColliderComponent {
    ColliderComponent(ColliderType colliderType, bool isStatic) : isStatic{ isStatic }
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


    ~ColliderComponent()
    {
        if (colliderShape)
        {
            delete colliderShape;
        }
    }

    ColliderShape* GetColliderShape() const
    {
        return colliderShape;
    }

    bool isStatic;

private:
    ColliderShape* colliderShape;
};

#endif //PBL_COMPONENTS_H
