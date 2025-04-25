#include "TransformSystem.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>
#include "Scene.h"


TransformSystem::TransformSystem(Scene* scene) : scene(scene) {}

void TransformSystem::updateNode(EntityID id) const {
    auto& transform = scene->GetComponent<Transform>(id);

    if (transform.isDirty) {
        if (transform.parent != -1) {
            auto& parentTransform = scene->GetComponent<Transform>(transform.parent);
            transform.globalMatrix = parentTransform.globalMatrix;
        }
        transform.globalMatrix = glm::translate(transform.globalMatrix, transform.translation);
        transform.globalMatrix *= glm::mat4_cast(transform.rotation);
        transform.globalMatrix = glm::scale(transform.globalMatrix, transform.scale);

        transform.isDirty = false;
    }
}

void TransformSystem::updateNodeRecursive(EntityID id) const {
    updateNode(id);
    for (auto child : scene->GetComponent<Transform>(id).children) {
        updateNodeRecursive(child);
    }
}

void TransformSystem::update() const {
    updateNodeRecursive(scene->GetSceneRootEntity());
}

void TransformSystem::translateEntity(EntityID id, const glm::vec3& translation) const {
    auto& transform = scene->GetComponent<Transform>(id);
    transform.translation = translation;
    markDirty(id);
}

void TransformSystem::rotateEntity(EntityID id, const glm::quat& rotation) const {
    auto& transform = scene->GetComponent<Transform>(id);
    transform.rotation = rotation;
    transform.eulerRotation = glm::degrees(glm::eulerAngles(rotation));
    markDirty(id);
}

void TransformSystem::rotateEntity(EntityID id, const glm::vec3& eulerRotation) const {
    auto& transform = scene->GetComponent<Transform>(id);
    transform.eulerRotation = eulerRotation;
    transform.rotation = glm::quat(glm::radians(eulerRotation));
    markDirty(id);
}

void TransformSystem::scaleEntity(EntityID id, const glm::vec3& scale) const {
    auto& transform = scene->GetComponent<Transform>(id);
    transform.scale = scale;
    markDirty(id);
}

void TransformSystem::setGlobalMatrix(EntityID id, const glm::mat4& mat) const {
    auto& transform = scene->GetComponent<Transform>(id);
    transform.globalMatrix = mat;

    glm::mat4 parentMatrix = glm::mat4(1.0);
    if (transform.parent != -1) {
        parentMatrix = scene->GetComponent<Transform>(transform.parent).globalMatrix;
    }


    glm::mat4 localMatrix = glm::inverse(parentMatrix) * mat;


    glm::vec3 position = glm::vec3(localMatrix[3]);
    localMatrix[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

    glm::vec3 scale;
    for (int i = 0; i < 3; ++i) {
        scale[i] = glm::length(glm::vec3(localMatrix[i]));
    }

    for (int i = 0; i < 3; ++i) {
        if (scale[i] != 0.0f) {
            localMatrix[i] /= scale[i];
        }
    }
    glm::vec3 rotation;
    glm::extractEulerAngleXYZ(localMatrix, rotation.x, rotation.y, rotation.z);


    transform.translation = position;
    transform.eulerRotation = glm::degrees(rotation);
    transform.rotation = glm::quat(rotation);
    transform.scale = scale;

    markDirty(id);
}


void TransformSystem::markDirty(EntityID id) const {
    auto& transform = scene->GetComponent<Transform>(id);
    transform.isDirty = true;
    for (auto child : transform.children) {
        markDirty(child);
    }
}


void TransformSystem::addChild(EntityID parent, EntityID child) const {
    auto& parentTransform = scene->GetComponent<Transform>(parent);
    auto& childTransform = scene->GetComponent<Transform>(child);

    if (childTransform.parent == parent)
        return;

    EntityID ancestor = childTransform.parent;
    while (ancestor != -1) {
        if (ancestor == child) {
            return;
        }

        auto& ancestorTransform = scene->GetComponent<Transform>(ancestor);
        ancestor = ancestorTransform.parent;
    }

    if (childTransform.parent != -1) {
        removeChild(childTransform.parent, child);
    }

    parentTransform.children.push_back(child);
    childTransform.parent = parent;

    setGlobalMatrix(child, childTransform.globalMatrix);
}

void TransformSystem::removeChild(EntityID parent, EntityID child) const {
    auto& parentTransform = scene->GetComponent<Transform>(parent);
    auto& childTransform = scene->GetComponent<Transform>(child);

    if (childTransform.parent != parent)
        return;

    std::erase(parentTransform.children, child);
    childTransform.parent = (EntityID) -1;
}