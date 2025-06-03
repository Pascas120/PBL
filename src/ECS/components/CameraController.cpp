//
// Created by lukas on 04.06.2025.
//

#include "CameraController.h"
#include "Scene.h"

void CameraController::update(GLFWwindow *window, Scene *scene, float deltaTime) {
    Transform targetTransform = scene->getComponent<Transform>(targetID);
    auto& transformSystem = scene->getTransformSystem();
    if (scene->hasComponent<CameraComponent>(id)) {
        auto& cameraComponent = scene->getComponent<CameraComponent>(id);
        glm::vec3 cameraPos = targetTransform.translation + offset;

        // Update camera position
        transformSystem.translateEntity(id, cameraPos);

        // // Optionally, you can also update the camera rotation to look at the target
        glm::vec3 direction = -glm::normalize(cameraPos - targetTransform.translation);
        glm::vec3 up = {0.0f, 1.0f, 0.0f}; // Assuming Y is up
        glm::vec3 right = glm::normalize(glm::cross(direction, up));
        up = glm::normalize(glm::cross(right, direction));

        transformSystem.rotateEntity(id, glm::quatLookAt(direction, up));

        if (cameraComponent.dirty) {
            cameraComponent.updateProjectionMatrix();
        }
    }
}
