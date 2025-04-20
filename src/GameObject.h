//
// Created by lukas on 31.03.2025.
//

#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H
#include "ComponentList.h"
#include "Shader.h"


constexpr int MAX_CHILDREN = 100;

class GameObject {
private:
    GameObject* parent;
    GameObject* children[MAX_CHILDREN];
    int childCount;
    bool dirty;

    int index = -1;

    std::string name;

public:
    ComponentList components;

    GameObject();
    void AddChild(GameObject* child);
	void RemoveChild(GameObject* child);
	void SetChildIndex(int oldIndex, int newIndex);
    void MarkDirty();
    void Update();
    void Draw(Shader& shader);

    GameObject* GetParent();
    GameObject** GetChildren();
    int GetChildCount();

    std::string GetName() const;
    int GetIndex() const;

    void SetName(const std::string& newName);

    static bool ChangeParent(GameObject* child, GameObject* newParent);
};



#endif //GAMEOBJECT_H
