//
// Created by lukas on 31.03.2025.
//

#ifndef COMPONENTLIST_H
#define COMPONENTLIST_H
#include <stdio.h>

#include "Component.h"
#define MAX_COMPONENTS 10
struct ComponentNode {
    Component* component;
    int next; // Indeks kolejnego komponentu (-1 = brak)
};

// Lista komponentów zarządzana przez indeksy
class ComponentList {
private:
    ComponentNode nodes[MAX_COMPONENTS]; // Tablica komponentów
    int head, tail;
    int freeIndex; // Indeks pierwszego wolnego miejsca

public:
    ComponentList();

    template<typename T>
    T* AddComponent();

    void RemoveComponent(Component* component);
    void Update();

    template<typename T>
    T* GetComponent();

    ~ComponentList();
};


#endif //COMPONENTLIST_H
