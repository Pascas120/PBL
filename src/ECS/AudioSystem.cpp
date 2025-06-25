//
// Created by lukas on 14.06.2025.
//

#include "AudioSystem.h"
#include "Scene.h"
#include "spdlog/spdlog.h"
#include "Application.h"


AudioSystem::AudioSystem(Scene *scene): scene(scene) {
    audioEngine = &scene->getApplication()->audioEngine;
}

AudioSystem::~AudioSystem() {
    stopAllSounds();
}

void AudioSystem::update() const {

}

void AudioSystem::playSound(EntityID id) {
    auto sounds = scene->getStorage<SoundComponent>();
    if (sounds == nullptr) {
        spdlog::warn("AudioSystem: No SoundComponent storage found in the scene");
        return;
    }
    if (sounds->has(id)) {
        auto& soundComponent = sounds->get(id);
        if (!soundComponent.isInitialized) {
            if(soundComponent.soundPath.empty()) {
                spdlog::warn("AudioSystem: Sound path is empty for entity {}", id);
                return;
            }
            ma_result result;
            if(soundComponent.loop) {
                result = ma_sound_init_from_file(
                audioEngine,
                soundComponent.soundPath.c_str(),
                MA_SOUND_FLAG_LOOPING,
                nullptr,
                nullptr,
                &soundComponent.sound
                );
            }else{
            result = ma_sound_init_from_file(
                audioEngine,
                soundComponent.soundPath.c_str(),
                0, // No flags
                nullptr,
                nullptr,
                &soundComponent.sound
            );
            }
            if (result != MA_SUCCESS) {
                spdlog::error("AudioSystem: Failed to initialize sound: {}", ma_result_description(result));
                return;
            }
            soundComponent.isInitialized = true;
        }
        ma_sound_set_volume(&soundComponent.sound, soundComponent.volume * globalVolume);
        ma_sound_start(&soundComponent.sound);
    } else {
        spdlog::warn("AudioSystem: Entity {} does not have a SoundComponent", id);
    }
}

void AudioSystem::playSound(std::string soundPath) {
    if (soundPath.empty()) {
        spdlog::warn("AudioSystem: Sound path is empty");
        return;
    }

    ma_result result = ma_engine_play_sound(
        audioEngine,
        soundPath.c_str(),
        nullptr // ma_sound_config can be passed here for volume, etc., or set after
    );

    // If you need to set volume:
    // ma_sound* pSound = ma_engine_play_sound_ex(&audioEngine, soundPath.c_str(), nullptr);
    // if (pSound) {
    //     ma_sound_set_volume(pSound, globalVolume);
    // }

    if (result != MA_SUCCESS) {
        spdlog::error("AudioSystem: Failed to play sound with ma_engine_play_sound: {}", ma_result_description(result));
        return;
    }
}

void AudioSystem::stopSound(EntityID id) const {
    auto sounds = scene->getStorage<SoundComponent>();
    if (sounds->has(id)) {
        auto& soundComponent = sounds->get(id);
        if (soundComponent.isInitialized) {
            ma_sound_stop(&soundComponent.sound);
        }
    } else {
        spdlog::warn("AudioSystem: Entity {} does not have a SoundComponent", id);
    }
}

void AudioSystem::setGlobalVolume(float volume) {
    globalVolume = std::clamp(volume, 0.0f, 1.0f);
}

float AudioSystem::getGlobalVolume() const {
    return globalVolume;
}

void AudioSystem::stopAllSounds() const {
    auto sounds = scene->getStorage<SoundComponent>();
    if(sounds == nullptr) {
        return;
    }
    for (int i = 0; i < sounds->getQuantity(); ++i) {
        auto& soundComponent = sounds->components[i];
        if (soundComponent.isInitialized) {
            ma_sound_stop(&soundComponent.sound);
            ma_sound_uninit(&soundComponent.sound);
        }
    }
}