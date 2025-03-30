//
// Created by lukas on 31.03.2025.
//

#include "Transform.h"

Transform::Transform()
    : translate(0.0f), rotate(0.0f), scale(1.0f), modelMatrix(1.0f) {}

void Transform::SetTranslation(const glm::vec3& t) { translate = t; }
void Transform::SetRotation(const glm::vec3& r) { rotate = r; }
void Transform::SetScale(const glm::vec3& s) { scale = s; }

const glm::vec3& Transform::GetTranslation() const { return translate; }
const glm::vec3& Transform::GetRotation() const { return rotate; }
const glm::vec3& Transform::GetScale() const { return scale; }
const glm::mat4& Transform::GetModelMatrix() const { return modelMatrix; }

void Transform::update() {
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, translate);
    modelMatrix = glm::rotate(modelMatrix, glm::radians(rotate.x), glm::vec3(1, 0, 0));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(rotate.y), glm::vec3(0, 1, 0));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(rotate.z), glm::vec3(0, 0, 1));
    modelMatrix = glm::scale(modelMatrix, scale);
}