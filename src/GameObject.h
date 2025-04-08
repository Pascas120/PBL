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
    bool dirty;

protected:
    int childCount;
    GameObject* children[MAX_CHILDREN];
public:
    ComponentList components;

    GameObject();
    void addChild(GameObject* child);
    void markDirty();
    void update();
    virtual void draw(Shader& shader);

    GameObject* GetParent();
    GameObject** GetChildren();
    int GetChildCount();
};



#endif //GAMEOBJECT_H
