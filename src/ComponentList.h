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
    ComponentList() : head(-1), tail(-1), freeIndex(0) {
        // Inicjalizacja listy wolnych miejsc
        for (int i = 0; i < MAX_COMPONENTS - 1; ++i)
            nodes[i].next = i + 1;
        nodes[MAX_COMPONENTS - 1].next = -1;
    }

    template<typename T>
    void AddComponent() {
        if (freeIndex == -1) {
            printf("No space for more components!\n");
            return;
        }

        int newIndex = freeIndex;
        freeIndex = nodes[newIndex].next; // Pobierz następne wolne miejsce

        nodes[newIndex].component = new T();
        nodes[newIndex].next = -1;

        if (head == -1) {
            head = tail = newIndex; // Pierwszy element
        } else {
            nodes[tail].next = newIndex;
            tail = newIndex;
        }
    }

    void Update() {
        int current = head;
        while (current != -1) {
            nodes[current].component->update();
            current = nodes[current].next;
        }
    }

    ~ComponentList() {
        int current = head;
        while (current != -1) {
            delete nodes[current].component;
            current = nodes[current].next;
        }
    }
};



#endif //COMPONENTLIST_H
