//
// Created by lukas on 26.06.2025.
//

#ifndef VIGNETTECONTROLLER_H
#define VIGNETTECONTROLLER_H
#include "ECS/Components.h"

class Scene;
struct VignetteController {

    EntityID id = (EntityID)-1;
    bool vignetteEnabled = false;
    void update(GLFWwindow* window, Scene* scene, float deltaTime);
};



#endif //VIGNETTECONTROLLER_H
