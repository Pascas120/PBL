//
// Created by Łukasz Moskwin on 15/04/2025.
//

#ifndef PBL_TRANSFORMSYSTEM_H
#define PBL_TRANSFORMSYSTEM_H


#include "SceneGraph.h"
#include "Components.h"
#include "Scene.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"

class TransformSystem {
private:
    Scene* scene;

    void updateNode(EntityID id) {
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


    void updateNodeRecursive(EntityID id) {
        updateNode(id);
        auto& node = scene->GetSceneGraph()->getNode(id);
        for (auto child : node.getChildren()) {
            updateNodeRecursive(child);
        }
    }
public:
    TransformSystem(Scene* scene) : scene(scene) {}

    void update() {
        auto root = scene->GetSceneGraph()->GetRoot();
        updateNodeRecursive(root);
    }

    void translateEntity(EntityID id, const glm::vec3& translation) {
        auto& transform = scene->GetComponent<Transform>(id);
        transform.translation = translation;
        scene->GetSceneGraph()->markDirty(id);
    }
    void rotateEntity(EntityID id, const glm::quat& rotation) {
        auto& transform = scene->GetComponent<Transform>(id);
        transform.rotation = rotation;
        scene->GetSceneGraph()->markDirty(id);
    }
    void scaleEntity(EntityID id, const glm::vec3& scale) {
        auto& transform = scene->GetComponent<Transform>(id);
        transform.scale = scale;
        scene->GetSceneGraph()->markDirty(id);
    }
};


#endif //PBL_TRANSFORMSYSTEM_H
