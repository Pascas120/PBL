//
// Created by lukas on 31.03.2025.
//

#ifndef SCENE_H
#define SCENE_H
#include "ECS/Components.h"

constexpr int MAX_OBJECTS = 100;

struct Scene {
Transform transforms[MAX_OBJECTS] = {};
uint8_t transformCount = 0;
ModelComponent modelComponents[MAX_OBJECTS] = {};
uint8_t modelComponentCount = 0;
ImageComponent imageComponents[MAX_OBJECTS] = {};
uint8_t imageComponentCount = 0;
TextComponent textComponents[MAX_OBJECTS] = {};
uint8_t textComponentCount = 0;
};



#endif //SCENE_H
