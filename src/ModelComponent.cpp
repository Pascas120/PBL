//
// Created by lukas on 31.03.2025.
//

#include "ModelComponent.h"

ModelComponent::ModelComponent(const std::string& modelPath, bool gamma)
    : model(modelPath, gamma) {}

ModelComponent::ModelComponent(Model* model) {
    this->model = *model;
}

void ModelComponent::Draw() {
	if (shader) {
		model.Draw(*shader);
	}
}

void ModelComponent::update() {
}

Model& ModelComponent::GetModel() {
    return model;
}