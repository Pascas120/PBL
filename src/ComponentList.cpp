//
// Created by lukas on 31.03.2025.
//

#include "ComponentList.h"

ComponentList::ComponentList() : head(-1), tail(-1), freeIndex(0) {
    // Inicjalizacja listy wolnych miejsc
    for (int i = 0; i < MAX_COMPONENTS - 1; ++i)
        nodes[i].next = i + 1;
    nodes[MAX_COMPONENTS - 1].next = -1;
}

void ComponentList::RemoveComponent(Component* component) {
    int current = head;
    int prev = -1;

    while (current != -1) {
        if (nodes[current].component == component) {
            delete nodes[current].component;
            nodes[current].component = nullptr;

            if (prev == -1) {
                head = nodes[current].next;
            } else {
                nodes[prev].next = nodes[current].next;
            }

            if (tail == current) {
                tail = prev;
            }

            nodes[current].next = freeIndex;
            freeIndex = current;

            return;
        }
        prev = current;
        current = nodes[current].next;
    }
}

void ComponentList::Update() {
    int current = head;
    while (current != -1) {
        nodes[current].component->update();
        current = nodes[current].next;
    }
}


ComponentList::~ComponentList() {
    int current = head;
    while (current != -1) {
        delete nodes[current].component;
        current = nodes[current].next;
    }
}