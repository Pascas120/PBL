//
// Created by lukas on 04.06.2025.
//

#ifndef CAMERACONTROLLER_H
#define CAMERACONTROLLER_H
#include "glfw/glfw3.h"
#include "ECS/EntityManager.h"
#include "glm/vec3.hpp"


class Scene;


struct CameraController
{
    EntityID targetID;
    glm::vec3 offset = {0.0f, 2.0f, 5.0f};

    void update(GLFWwindow* window, Scene* scene, float deltaTime);


    EntityID id = (EntityID)-1;
};


#endif //CAMERACONTROLLER_H
