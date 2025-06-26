//
// Created by lukas on 26.06.2025.
//

#include "VignetteController.h"
#include "Scene.h"

void VignetteController::update(GLFWwindow* window, Scene* scene, float deltaTime)
{
    auto& image = scene->getComponent<ImageComponent>(id);
    if(vignetteEnabled) {
        if(image.color.a < 0.78) {
            image.color.a += 0.7 * deltaTime;
            if(image.color.a > 0.78) {
                image.color.a = 0.78;
            }
        }
    }
    else {
        if(image.color.a > 0) {
            image.color.a -= 2 * deltaTime;
            if(image.color.a < 0) {
                image.color.a = 0;
            }
        }
    }

    if(image.color.a > 0.78) {
        image.color.a = 0.78;
    }
    if(image.color.a < 0) {
        image.color.a = 0;
    }
}