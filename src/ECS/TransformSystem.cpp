#include "TransformSystem.h"

TransformSystem::TransformSystem(Scene* scene) : scene(scene) {}

void TransformSystem::updateNode(EntityID id) {
    auto& node = scene->GetSceneGraph()->getNode(id);
    auto& transform = scene->GetComponent<Transform>(id);

    if (scene->GetSceneGraph()->getNode(id).isDirty(id)) {
        if (node.getParent() != -1) {
            auto& parentTransform = scene->GetComponent<Transform>(node.getParent());
            transform.globalMatrix = parentTransform.globalMatrix *
                                     glm::translate(glm::mat4(1.0f), transform.translation) *
                                     glm::mat4_cast(transform.rotation) *
                                     glm::scale(glm::mat4(1.0f), transform.scale);
        } else {
            transform.globalMatrix = glm::translate(glm::mat4(1.0f), transform.translation) *
                                     glm::mat4_cast(transform.rotation) *
                                     glm::scale(glm::mat4(1.0f), transform.scale);
        }

        scene->GetSceneGraph()->getNode(id).setDirty(false);
    }
}

void TransformSystem::updateNodeRecursive(EntityID id) {
    updateNode(id);
    auto& node = scene->GetSceneGraph()->getNode(id);
    for (auto child : node.getChildren()) {
        updateNodeRecursive(child);
    }
}

void TransformSystem::update() {
    auto root = scene->GetSceneGraph()->GetRoot();
    updateNodeRecursive(root);
}

void TransformSystem::translateEntity(EntityID id, const glm::vec3& translation) {
    auto& transform = scene->GetComponent<Transform>(id);
    transform.translation = translation;
    scene->GetSceneGraph()->markDirty(id);
}

void TransformSystem::rotateEntity(EntityID id, const glm::quat& rotation) {
    auto& transform = scene->GetComponent<Transform>(id);
    transform.rotation = rotation;
    scene->GetSceneGraph()->markDirty(id);
}

void TransformSystem::scaleEntity(EntityID id, const glm::vec3& scale) {
    auto& transform = scene->GetComponent<Transform>(id);
    transform.scale = scale;
    scene->GetSceneGraph()->markDirty(id);
}