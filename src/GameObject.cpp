//
// Created by lukas on 31.03.2025.
//

#include "GameObject.h"

GameObject::GameObject() : parent(nullptr), childCount(0), dirty(true) {
    for (int i = 0; i < MAX_CHILDREN; i++) {
        children[i] = nullptr;
    }
}

// Dodaje dziecko do GameObject
void GameObject::AddChild(GameObject* child) {
    if (childCount >= MAX_CHILDREN) return;
    children[childCount++] = child;
    child->parent = this;
}

// Oznacza obiekt jako brudny i propaguje w dół
void GameObject::MarkDirty() {
    dirty = true;
    for (int i = 0; i < childCount; i++) {
        if (children[i]) {
            children[i]->MarkDirty();
        }
    }
}

// Aktualizuje tylko brudne węzły
void GameObject::Update() {
    if (dirty) {
        components.Update();
        dirty = false;
    }
    for (int i = 0; i < childCount; i++) {
        if (children[i]) {
            children[i]->Update();
        }
    }
}

GameObject* GameObject::GetParent() {
    return parent;
}

GameObject** GameObject::GetChildren() {
    return children;
}

int GameObject::GetChildCount() {
    return childCount;
}
