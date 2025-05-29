//
// Created by Łukasz Moskwin on 14/04/2025.
//

#ifndef PBL_COMPONENTS_H
#define PBL_COMPONENTS_H
#include <array>

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include <string>
#include "Model.h"
#include "EntityManager.h"
#include "ColliderShape.h"
#include <memory>

#include "ECS/BoundingVolumes.h"
#include "Camera.h"

#include "components/CameraComponent.h"

enum class FlyVariant : uint8_t {
    GREEN = 0,
    RED = 1,
    GOLD = 2,
    PURPLE = 3,
    COUNT
};

class Model;

struct ObjectInfoComponent {
    std::string name;
    std::string uuid;
    std::string tag;

	EntityID id;
};

struct Transform {
    bool isStatic = true;

    glm::vec3 translation = {0.0f, 0.0f, 0.0f};
    glm::quat rotation = {1.0f, 0.0f, 0.0f, 0.0f};
    glm::vec3 eulerRotation = glm::vec3(0.0f);
    glm::vec3 scale = {1.0f, 1.0f, 1.0f};
    glm::mat4 globalMatrix = glm::mat4(1.0f);

    bool isDirty = true;
    std::vector<EntityID> children;
    EntityID parent = (EntityID) -1;

    std::string uuid;

	EntityID id = (EntityID)-1;
};

struct ModelComponent {
    Shader* shader;
    Model* model;
    Transform* transform;

	glm::mat4 prevModelMatrix = glm::mat4(1.0f);

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

struct PointLightComponent {
    glm::vec3 color = { 1.0f, 1.0f, 1.0f };
    float intensity = 1.0f;

    float constant = 1.0f;
    float linear = 0.09f;
    float quadratic = 0.032f;

    EntityID id = (EntityID)-1;
};

struct DirectionalLightComponent {
    glm::vec3 color = { 1.0f, 1.0f, 1.0f };
    float intensity = 1.0f;

    EntityID id = (EntityID)-1;
};

struct FlyAIComponent {
	EntityID idButter = (EntityID)-1;
    EntityID idBread = (EntityID)-1;
    float patrolHeightOffset = 3.f;
    float patrolSpeed = 3.f;
    float diveSpeed = 6.f;
    float detectionRadius = 8.f;
    float diveEndHeight = 0.4f;
    float returnSpeed = 3.5f;
    float patrolRange = 10.f;
    float patrolPointReachedThreshold = 0.5f;
    float diveCooldownTime = 2.f;
    float diveCooldownTimer = 0.f;
	float groundY = 0.f;
    enum FlyState { Patrolling, Diving, Returning }
    state = FlyState::Patrolling;
    glm::vec3 patrolTarget;

    EntityID id = (EntityID)-1;


};
#endif //PBL_COMPONENTS_H
