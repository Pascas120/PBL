//
// Created by lukas on 31.03.2025.
//

#include "Scene.h"
#include "Transform.h"

Scene::Scene(){
    root = new GameObject();
}

// Tworzy nowy GameObject w scenie
void Scene::addChild(GameObject *obj){
    root->addChild(obj);
}

void Scene::Update() {
    root->update();
}

void Scene::Draw(Shader& shader) {
    root->draw(shader);
}