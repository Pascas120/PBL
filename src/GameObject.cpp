//
// Created by lukas on 31.03.2025.
//

#include "GameObject.h"

#include "ModelComponent.h"
#include "Transform.h"

GameObject::GameObject() : parent(nullptr), childCount(0), dirty(true) {
    for (int i = 0; i < MAX_CHILDREN; i++) {
        children[i] = nullptr;
    }
}

void GameObject::addChild(GameObject* child) {
    if (childCount >= MAX_CHILDREN) return;

    if (child->parent) {
        GameObject* oldParent = child->parent;
        for (int i = 0; i < oldParent->childCount; i++) {
            if (oldParent->children[i] == child) {
                for (int j = i; j < oldParent->childCount - 1; j++) {
                    oldParent->children[j] = oldParent->children[j + 1];
                }
                oldParent->childCount--;
                break;
            }
        }
    }

    children[childCount++] = child;
    child->parent = this;
}

void GameObject::markDirty() {
    dirty = true;
    for (int i = 0; i < childCount; i++) {
        if (children[i]) {
            children[i]->markDirty();
        }
    }
}

void GameObject::update() {
    if (dirty) {
        if (parent) {
            Transform* parentTransform = parent->components.GetComponent<Transform>();
            Transform* transform = components.GetComponent<Transform>();
            if (parentTransform && transform) {
                transform->setParentMatrix(parentTransform->getModelMatrix());
            }
        }
        components.Update();
        dirty = false;
    }

    for (int i = 0; i < childCount; i++) {
        if (children[i]) {
            children[i]->update();
        }
    }
}

void GameObject::draw(Shader& shader) {
    ModelComponent* modelComp = components.GetComponent<ModelComponent>();
    Transform* transform = components.GetComponent<Transform>();
    if (modelComp) {
        shader.setMat4("model", transform->getModelMatrix());
        modelComp->Draw(shader);
    }

    for (int i = 0; i < childCount; i++) {
        if (children[i]) {
            children[i]->draw(shader);
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
