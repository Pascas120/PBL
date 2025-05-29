#include "TransformSystem.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>
#include "Scene.h"

static void continuousQuatToEuler(glm::vec3& eulerAngles, const glm::quat& quat)
{
	glm::vec3 newEuler = glm::degrees(glm::eulerAngles(quat));
	glm::vec3 delta = newEuler - eulerAngles;
	for (int i = 0; i < 3; ++i) {
		//while (delta[i] > 180.0f) delta[i] -= 360.0f;
		//while (delta[i] < -180.0f) delta[i] += 360.0f;
		delta[i] = std::fmod(delta[i] + 180.0f, 360.0f) - 180.0f;
	}

	int flippedIndex1 = -1, flippedIndex2 = -1;
	for (int i = 0; i < 3; ++i) {
		if (std::abs(std::abs(delta[i]) - 180.0f) < 1e-6f) {
			if (flippedIndex1 == -1) {
				flippedIndex1 = i;
			}
			else if (flippedIndex2 == -1) {
				flippedIndex2 = i;
			}
            else {
                flippedIndex1 = -1;
            }
		}
	}

	if (flippedIndex1 != -1 && flippedIndex2 != -1) {
		delta[flippedIndex1] = delta[flippedIndex2] = 0.0f;
		int otherIndex = 3 - flippedIndex1 - flippedIndex2;

		float unflipped = 180.0 - newEuler[otherIndex];
		delta[otherIndex] = std::fmod(unflipped - eulerAngles[otherIndex] + 180.0f, 360.0f) - 180.0f;

	}

	eulerAngles += delta;

}


TransformSystem::TransformSystem(Scene* scene) : scene(scene) {}

void TransformSystem::updateNode(EntityID id) const {
    auto& transform = scene->getComponent<Transform>(id);

    if (transform.isDirty) {
        if (transform.parent != (EntityID)-1) {
            auto& parentTransform = scene->getComponent<Transform>(transform.parent);
            transform.globalMatrix = parentTransform.globalMatrix;
        }
        else
        {
			transform.globalMatrix = glm::mat4(1.0f);
        }
        transform.globalMatrix = glm::translate(transform.globalMatrix, transform.translation);
        transform.globalMatrix *= glm::mat4_cast(transform.rotation);
        transform.globalMatrix = glm::scale(transform.globalMatrix, transform.scale);

        transform.isDirty = false;
    }
}

void TransformSystem::updateNodeRecursive(EntityID id) const {
    updateNode(id);
    for (auto child : scene->getComponent<Transform>(id).children) {
        updateNodeRecursive(child);
    }
}

void TransformSystem::update() const {
    updateNodeRecursive(scene->getSceneRootEntity());
}

void TransformSystem::translateEntity(EntityID id, const glm::vec3& translation) const {
    auto& transform = scene->getComponent<Transform>(id);
    transform.translation = translation;
    markDirty(id);
}

void TransformSystem::rotateEntity(EntityID id, const glm::quat& rotation) const {
    auto& transform = scene->getComponent<Transform>(id);
    transform.rotation = rotation;
    //transform.eulerRotation = glm::degrees(glm::eulerAngles(rotation));
	continuousQuatToEuler(transform.eulerRotation, rotation);
    markDirty(id);
}

void TransformSystem::rotateEntity(EntityID id, const glm::vec3& eulerRotation) const {
    auto& transform = scene->getComponent<Transform>(id);
    transform.eulerRotation = eulerRotation;
    transform.rotation = glm::quat(glm::radians(eulerRotation));
    markDirty(id);
}

void TransformSystem::rotateEntity(EntityID id, const glm::vec3& target, float delta) const {
    auto& transform = scene->getComponent<Transform>(id);
    glm::quat targetRotation = glm::quat(glm::radians(target));
    transform.rotation = glm::slerp(transform.rotation, targetRotation, delta);
    continuousQuatToEuler(transform.eulerRotation, transform.rotation);
    markDirty(id);
}

void TransformSystem::scaleEntity(EntityID id, const glm::vec3& scale) const {
    auto& transform = scene->getComponent<Transform>(id);
    transform.scale = scale;
    markDirty(id);
}

void TransformSystem::setGlobalMatrix(EntityID id, const glm::mat4& mat) const {
    auto& transform = scene->getComponent<Transform>(id);
    transform.globalMatrix = mat;

    glm::mat4 parentMatrix = glm::mat4(1.0);
    if (transform.parent != -1) {
        parentMatrix = scene->getComponent<Transform>(transform.parent).globalMatrix;
    }


    glm::mat4 localMatrix = glm::inverse(parentMatrix) * mat;


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
	glm::quat rotation = glm::quat_cast(localMatrix);



    transform.translation = position;
    //transform.eulerRotation = glm::degrees(glm::eulerAngles(rotation));
	continuousQuatToEuler(transform.eulerRotation, rotation);
    transform.rotation = rotation;
    transform.scale = scale;

    markDirty(id);
}


void TransformSystem::markDirty(EntityID id) const {
    auto& transform = scene->getComponent<Transform>(id);
    transform.isDirty = true;
    for (auto child : transform.children) {
        markDirty(child);
    }
}


bool TransformSystem::addChild(EntityID parent, EntityID child) const {
    auto& parentTransform = scene->getComponent<Transform>(parent);
    auto& childTransform = scene->getComponent<Transform>(child);

    if (childTransform.parent == parent)
        return false;

    EntityID ancestor = parentTransform.id;
	while (ancestor != scene->getSceneRootEntity() && ancestor != (EntityID)-1) {
        if (ancestor == child) {
            return false;
        }

        auto& ancestorTransform = scene->getComponent<Transform>(ancestor);
        ancestor = ancestorTransform.parent;
    }

    if (childTransform.parent != -1) {
        removeChild(childTransform.parent, child);
    }

    parentTransform.children.push_back(child);
    childTransform.parent = parent;

	return true;
}

void TransformSystem::addChildKeepTransform(EntityID parent, EntityID child) const {
	glm::mat4 globalMatrix = scene->getComponent<Transform>(child).globalMatrix;
	if (addChild(parent, child))
	    setGlobalMatrix(child, globalMatrix);
}

void TransformSystem::removeChild(EntityID parent, EntityID child) const {
    auto& parentTransform = scene->getComponent<Transform>(parent);
    auto& childTransform = scene->getComponent<Transform>(child);

    if (childTransform.parent != parent)
        return;

    std::erase(parentTransform.children, child);
    childTransform.parent = (EntityID) -1;
}

void TransformSystem::setChildIndex(EntityID child, int index) const {
	auto& childTransform = scene->getComponent<Transform>(child);
	auto& parentTransform = scene->getComponent<Transform>(childTransform.parent);

	int currentIndex = std::find(parentTransform.children.begin(), parentTransform.children.end(), child) - parentTransform.children.begin();
	if (currentIndex == index)
		return;

	if (currentIndex < index) {
		index--;
	}

	std::erase(parentTransform.children, child);
	parentTransform.children.insert(parentTransform.children.begin() + index, child);
}

int TransformSystem::getChildIndex(EntityID child) const {
    auto& childTransform = scene->getComponent<Transform>(child);
    auto& parentTransform = scene->getComponent<Transform>(childTransform.parent);

	return std::find(parentTransform.children.begin(), parentTransform.children.end(), child) - parentTransform.children.begin();
}