//
// Created by lukas on 31.03.2025.
//

#ifndef SCENE_H
#define SCENE_H


#include "GameObject.h"

constexpr int MAX_OBJECTS = 100;

class Scene {
private:
    GameObject objects[MAX_OBJECTS];
    int objectCount;
    GameObject* root;

public:
    Scene();

    GameObject* CreateGameObject();
    void Update();
    void Draw(Shader &shader);
};



#endif //SCENE_H
