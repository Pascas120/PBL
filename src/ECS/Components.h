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
#include <memory>

struct ObjectInfoComponent {
    std::string name;

	EntityID id;
};

struct Transform {
    glm::vec3 translation = {0.0f, 0.0f, 0.0f};
    glm::quat rotation = {1.0f, 0.0f, 0.0f, 0.0f};
    glm::vec3 eulerRotation = glm::vec3(0.0f);
    glm::vec3 scale = {1.0f, 1.0f, 1.0f};
    glm::mat4 globalMatrix = glm::mat4(1.0f);

    bool isDirty = false;
    std::vector<EntityID> children;
    EntityID parent = (EntityID) -1;

	EntityID id = (EntityID)-1;
};

struct ModelComponent {
    Shader* shader;
    Model* model;

    EntityID id = (EntityID)-1;
};

struct ImageComponent {
    Shader* shader;
    std::string texturePath;
    float width;
    float height;
    glm::vec4 color;

    EntityID id = (EntityID)-1;
};

struct TextComponent {
    Shader* shader;
    std::string font;
    glm::vec4 color;
    std::string text;

    EntityID id = (EntityID)-1;
};

struct ColliderComponent {
    ColliderComponent(ColliderType colliderType, bool isStatic = false) : isStatic{ isStatic }
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
	ColliderComponent() = default;

    // unsafe (TODO)
    ColliderShape* GetColliderShape() const
    {
        return colliderShape.get();
    }

    bool isStatic;


    EntityID id = (EntityID)-1;

private:
    std::shared_ptr<ColliderShape> colliderShape = nullptr;
};

#endif //PBL_COMPONENTS_H
