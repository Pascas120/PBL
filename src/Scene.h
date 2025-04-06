//
// Created by lukas on 31.03.2025.
//

#ifndef SCENE_H
#define SCENE_H


#include "GameObject.h"
#include "CollisionSystem.h"

constexpr int MAX_OBJECTS = 100;

class Scene {
private:
    GameObject* root;
    CollisionSystem collisionSystem = CollisionSystem(this);

public:
    Scene();

    void addChild(GameObject* obj);
    void Update();
    void Draw(Shader &shader);

	GameObject* GetRoot() {
		return root;
	}

	CollisionSystem* GetCollisionSystem() {
		return &collisionSystem;
	}
};



#endif //SCENE_H
