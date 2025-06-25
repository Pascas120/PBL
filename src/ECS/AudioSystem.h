//
// Created by lukas on 14.06.2025.
//

#ifndef AUDIOSYSTEM_H
#define AUDIOSYSTEM_H
#include "ECS/Components.h"
#include "miniaudio.h"

class Scene;

class AudioSystem {
    Scene* scene;
    ma_engine audioEngine;
    float globalVolume = 0.5f;
public:
    AudioSystem(Scene* scene);
    ~AudioSystem();

    void update() const;

    void playSound(EntityID id);
    void playSound(std::string soundPath);
    void stopSound(EntityID id) const;
    void setGlobalVolume(float volume);
    float getGlobalVolume() const;
    void stopAllSounds() const;
};



#endif //AUDIOSYSTEM_H
