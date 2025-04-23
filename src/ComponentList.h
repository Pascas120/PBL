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

    template<typename T, typename... Args>
    T* AddComponent(Args&&... args) {
        if (freeIndex == -1) {
            printf("No space for more components!\n");
            return nullptr;
        }

        int newIndex = freeIndex;
        freeIndex = nodes[newIndex].next;

        // Tworzenie komponentu z przekazanymi argumentami
        nodes[newIndex].component = new T(args...);
        nodes[newIndex].next = -1;

        if (head == -1) {
            head = tail = newIndex;
        } else {
            nodes[tail].next = newIndex;
            tail = newIndex;
        }

        return static_cast<T*>(nodes[newIndex].component);
    }

	// for serialization
	template<typename T>
	T* AddComponent(T* component) {
		if (freeIndex == -1) {
			printf("No space for more components!\n");
			return nullptr;
		}
		int newIndex = freeIndex;
		freeIndex = nodes[newIndex].next;
		nodes[newIndex].component = component;
		nodes[newIndex].next = -1;
		if (head == -1) {
			head = tail = newIndex;
		}
		else {
			nodes[tail].next = newIndex;
			tail = newIndex;
		}
		return static_cast<T*>(nodes[newIndex].component);
	}


    void RemoveComponent(Component* component);
    void Update();

    template<typename T>
    T* GetComponent() {
        int current = head;
        while (current != -1) {
            T* casted = dynamic_cast<T*>(nodes[current].component);
            if (casted) return casted;
            current = nodes[current].next;
        }
        return nullptr;
    }

    ~ComponentList();
};


#endif //COMPONENTLIST_H
