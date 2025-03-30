//
// Created by lukas on 31.03.2025.
//

#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H
#include "ComponentList.h"


constexpr int MAX_CHILDREN = 10;

class GameObject {
private:
    ComponentList components;
    GameObject* parent;
    GameObject* children[MAX_CHILDREN];
    int childCount;
    bool dirty;

public:
    GameObject();

    void AddChild(GameObject* child);
    void MarkDirty();
    void Update();

    GameObject* GetParent();
    GameObject** GetChildren();
    int GetChildCount();
};



#endif //GAMEOBJECT_H
