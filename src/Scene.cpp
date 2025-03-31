//
// Created by lukas on 31.03.2025.
//

#include "Scene.h"

Scene::Scene() : objectCount(0) {
    root = &objects[objectCount++];
}

// Tworzy nowy GameObject w scenie
GameObject* Scene::CreateGameObject() {
    if (objectCount >= MAX_OBJECTS) return nullptr;
    return &objects[objectCount++];
}

void Scene::Update() {
    root->Update();
}

void Scene::Draw(Shader& shader) {
    root->Draw(shader);
}