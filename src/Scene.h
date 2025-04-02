//
// Created by lukas on 31.03.2025.
//

#ifndef SCENE_H
#define SCENE_H


#include "GameObject.h"

constexpr int MAX_OBJECTS = 100;

class Scene {
private:
    GameObject* root;

public:
    Scene();

    void addChild(GameObject* obj);
    void Update();
    void Draw(Shader &shader);
};



#endif //SCENE_H
