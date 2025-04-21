//
// Created by lukas on 31.03.2025.
//

#include "GameObject.h"

#include "ModelComponent.h"
#include "Transform.h"
#include "glm/gtc/quaternion.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>
#include <algorithm>

GameObject::GameObject() : parent(nullptr), childCount(0), dirty(true) {
    for (int i = 0; i < MAX_CHILDREN; i++) {
        children[i] = nullptr;
    }
}

void GameObject::AddChild(GameObject* child) {
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
	child->index = childCount - 1;



	Transform* transform = components.GetComponent<Transform>();
	Transform* childTransform = child->components.GetComponent<Transform>();

	if (childTransform) {

        glm::mat4 parentMatrix = transform ? transform->getModelMatrix() : glm::mat4(1.0f);

		glm::mat4 childMatrix = childTransform->getModelMatrix();

		glm::mat4 localMatrix = glm::inverse(parentMatrix) * childMatrix;


		glm::vec3 position = glm::vec3(localMatrix[3]);
		localMatrix[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

		glm::vec3 scale;
		for (int i = 0; i < 3; ++i) {
			scale[i] = glm::length(glm::vec3(localMatrix[i]));
		}

		for (int i = 0; i < 3; ++i) {
            if (scale[i] != 0.0f) {
                localMatrix[i] /= scale[i];
            }
		}
        glm::vec3 rotation;
		glm::extractEulerAngleXYZ(localMatrix, rotation.x, rotation.y, rotation.z);


		childTransform->setTranslation(position);
		childTransform->setRotation(glm::degrees(rotation));
		childTransform->setScale(scale);

		childTransform->setParentMatrix(parentMatrix);
		child->MarkDirty();
	}
}

void GameObject::RemoveChild(GameObject* child) {

	for (int i = 0; i < childCount; i++) {
		if (children[i] == child) {
			for (int j = i; j < childCount - 1; j++) {
				children[j] = children[j + 1];
			}
			children[--childCount] = nullptr;
			child->parent = nullptr;
			child->index = -1;
			break;
		}
	}
}

void GameObject::SetChildIndex(int oldIndex, int newIndex) {
	if (oldIndex < 0 || oldIndex >= childCount || oldIndex == newIndex) {
		return;
	}

	newIndex = std::clamp(newIndex, 0, childCount - 1);

	GameObject* child = children[oldIndex];
	if (oldIndex < newIndex) {
		std::move(children + oldIndex + 1, children + newIndex + 1, children + oldIndex);
	}
    else {
        std::move_backward(children + newIndex, children + oldIndex, children + oldIndex + 1);
    }
	children[newIndex] = child;
	child->index = newIndex;
}

void GameObject::MarkDirty() {
    dirty = true;
    for (int i = 0; i < childCount; i++) {
        if (children[i]) {
            children[i]->MarkDirty();
        }
    }
}

void GameObject::Update() {
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
            children[i]->Update();
        }
    }
}

void GameObject::Draw() {
    ModelComponent* modelComp = components.GetComponent<ModelComponent>();
    Transform* transform = components.GetComponent<Transform>();
    if (modelComp) {
		Shader* shader = modelComp->getShader();
		shader->use();
        shader->setMat4("model", transform->getModelMatrix());
        modelComp->Draw();
    }

    for (int i = 0; i < childCount; i++) {
        if (children[i]) {
            children[i]->Draw();
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

std::string GameObject::GetName() const {
	return name;
}

int GameObject::GetIndex() const {
	return index;
}

void GameObject::SetName(const std::string& newName) {
	name = newName;
}

bool GameObject::ChangeParent(GameObject* child, GameObject* newParent) {
	if (newParent->GetChildCount() >= MAX_CHILDREN)
		return false;

    GameObject* parentTree = newParent;
	while (parentTree) {
		if (parentTree == child)
			return false;

		parentTree = parentTree->GetParent();
	}
    
	GameObject* oldParent = child->GetParent();
	if (oldParent) {
		oldParent->RemoveChild(child);
	}
	newParent->AddChild(child);

	return true;
}
