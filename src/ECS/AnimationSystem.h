//
// Created by lukas on 21.06.2025.
//

#ifndef ANIMATIONSYSTEM_H
#define ANIMATIONSYSTEM_H
#include "Components.h"

class Scene;

class AnimationSystem {
private:
    Scene* scene;
public:
    AnimationSystem(Scene* scene) : scene(scene) {}

    void update(float deltaTime);

    void playCurrentAnimation(EntityID entityId);
    void playAnimation(EntityID entityId, Animation* animation);
    void playAnimation(EntityID entityId, const std::string& animationPath);
    void stopAnimation(EntityID entityId);
};



#endif //ANIMATIONSYSTEM_H
