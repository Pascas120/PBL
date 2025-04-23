#include "TransformSystem.h"

TransformSystem::TransformSystem(Scene* scene) : scene(scene) {}

void TransformSystem::updateNode(EntityID id) {
    auto& transform = scene->GetComponent<Transform>(id);

    if (transform.isDirty) {
        if (transform.parent != -1) {
            auto& parentTransform = scene->GetComponent<Transform>(transform.parent);
            transform.globalMatrix = parentTransform.globalMatrix *
                                     glm::translate(glm::mat4(1.0f), transform.translation) *
                                     glm::mat4_cast(transform.rotation) *
                                     glm::scale(glm::mat4(1.0f), transform.scale);
        } else {
            transform.globalMatrix = glm::translate(glm::mat4(1.0f), transform.translation) *
                                     glm::mat4_cast(transform.rotation) *
                                     glm::scale(glm::mat4(1.0f), transform.scale);
        }

        transform.isDirty = false;
    }
}

void TransformSystem::updateNodeRecursive(EntityID id) {
    updateNode(id);
    for (auto child : scene->GetComponent<Transform>(id).children) {
        updateNodeRecursive(child);
    }
}

void TransformSystem::update() {
    updateNodeRecursive(scene->GetSceneRootEntity());
}

void TransformSystem::translateEntity(EntityID id, const glm::vec3& translation) {
    auto& transform = scene->GetComponent<Transform>(id);
    transform.translation = translation;
    markDirty(id);
}

void TransformSystem::rotateEntity(EntityID id, const glm::quat& rotation) {
    auto& transform = scene->GetComponent<Transform>(id);
    transform.rotation = rotation;
    markDirty(id);
}

void TransformSystem::scaleEntity(EntityID id, const glm::vec3& scale) {
    auto& transform = scene->GetComponent<Transform>(id);
    transform.scale = scale;
    markDirty(id);
}

void TransformSystem::markDirty(EntityID id){
    auto& transform = scene->GetComponent<Transform>(id);
    transform.isDirty = true;
    for (auto child : transform.children) {
        markDirty(child);
    }
}