//
// Created by lukas on 31.03.2025.
//

#include "Transform.h"

Transform::Transform()
    : translate(0.0f), rotate(0.0f), scale(1.0f), modelMatrix(1.0f) {}

void Transform::setTranslation(const glm::vec3& t) { translate = t; }
void Transform::setRotation(const glm::vec3& r) { rotate = r; }
void Transform::setScale(const glm::vec3& s) { scale = s; }
void Transform::setParentMatrix(const glm::mat4 &m){parentMatrix = m;}

const glm::vec3& Transform::getTranslation() const { return translate; }
const glm::vec3& Transform::getRotation() const { return rotate; }
const glm::vec3& Transform::getScale() const { return scale; }
const glm::mat4& Transform::getModelMatrix() const { return modelMatrix; }

void Transform::update() {
    modelMatrix = parentMatrix;
    modelMatrix = glm::translate(modelMatrix, translate);
    modelMatrix = glm::rotate(modelMatrix, glm::radians(rotate.x), glm::vec3(1, 0, 0));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(rotate.y), glm::vec3(0, 1, 0));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(rotate.z), glm::vec3(0, 0, 1));
    modelMatrix = glm::scale(modelMatrix, scale);
}