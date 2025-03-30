//
// Created by lukas on 31.03.2025.
//

#include "ModelComponent.h"

ModelComponent::ModelComponent(const std::string& modelPath, bool gamma)
    : model(modelPath, gamma) {}

void ModelComponent::Draw(Shader& shader) {
    model.Draw(shader);
}

void ModelComponent::Update() {
}

Model& ModelComponent::GetModel() {
    return model;
}