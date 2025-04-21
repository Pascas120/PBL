//
// Created by lukas on 31.03.2025.
//

#ifndef MODELCOMPONENT_H
#define MODELCOMPONENT_H

#include "Component.h"
#include "Model.h"

class ModelComponent : public Component {
private:
    Model model;
	Shader* shader = nullptr;

public:
    ModelComponent(const std::string& modelPath, bool gamma = false);
    ModelComponent(Model* model);

    void Draw();
    void update() override;

	void setShader(Shader* shader) { this->shader = shader; }
	Shader* getShader() { return shader; }

    Model& GetModel();
};

#endif //MODELCOMPONENT_H
