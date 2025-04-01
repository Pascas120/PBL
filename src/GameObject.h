//
// Created by lukas on 31.03.2025.
//

#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H
#include "ComponentList.h"
#include "Shader.h"


constexpr int MAX_CHILDREN = 10;

class GameObject {
private:
    GameObject* parent;
    GameObject* children[MAX_CHILDREN];
    int childCount;
    bool dirty;

public:
    ComponentList components;

    GameObject();
    void AddChild(GameObject* child);
    void MarkDirty();
    void Update();
    void Draw(Shader& shader);

    GameObject* GetParent();
    GameObject** GetChildren();
    int GetChildCount();
};



#endif //GAMEOBJECT_H
