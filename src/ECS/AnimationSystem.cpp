//
// Created by lukas on 21.06.2025.
//

#include "AnimationSystem.h"
#include "Scene.h"

void AnimationSystem::update(float deltaTime)
{
    auto animations = scene->getStorage<AnimationComponent>();
    if (!animations)
        return;
    for (int i = 0; i < animations->getQuantity(); i++)
    {
        auto& animation = animations->components[i];
        animation.animator->SetLoop(animation.loop);
        animation.isPlaying = animation.animator && animation.animator->IsPlaying();
        if (animation.isPlaying && animation.animator)
        {
            animation.animator->UpdateAnimation(deltaTime * animation.playbackSpeed);
        }
    }
}

void AnimationSystem::playAnimation(EntityID entityId, Animation* animation)
{
    if (scene->hasComponent<AnimationComponent>(entityId))
    {
        auto& animationComponent = scene->getComponent<AnimationComponent>(entityId);
        if (animationComponent.animator)
        {
            animationComponent.isPlaying = true;
            animationComponent.animator->PlayAnimation(animation);
        }
    }
}

void AnimationSystem::playAnimation(EntityID entityId, const std::string &animationPath)
{
    if (scene->hasComponent<AnimationComponent>(entityId))
    {
        auto& animationComponent = scene->getComponent<AnimationComponent>(entityId);
        if (animationComponent.animator)
        {
            animationComponent.isPlaying = true;
            Animation* animation = nullptr;
            for (auto& anim : animationComponent.animations)
            {
                if (anim->path == animationPath)
                {
                    animation = anim;
                    break;
                }
            }
            animationComponent.animator->PlayAnimation(animation);
        }
    }
}

void AnimationSystem::playCurrentAnimation(EntityID entityId)
{
    if (scene->hasComponent<AnimationComponent>(entityId))
    {
        auto& animationComponent = scene->getComponent<AnimationComponent>(entityId);
        if (animationComponent.animator)
        {
            animationComponent.isPlaying = true;
            animationComponent.animator->PlayCurrentAnimation();
        }
    }
}

void AnimationSystem::stopAnimation(EntityID entityId)
{
    if (scene->hasComponent<AnimationComponent>(entityId))
    {
        auto& animationComponent = scene->getComponent<AnimationComponent>(entityId);
        animationComponent.animator->PlayAnimation(nullptr);
        animationComponent.isPlaying = false;
    }
}