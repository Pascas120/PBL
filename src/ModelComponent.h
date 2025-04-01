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

public:
    ModelComponent(const std::string& modelPath, bool gamma = false);
    ModelComponent(Model* model);

    void Draw(Shader& shader);
    void update() override;

    Model& GetModel();
};

#endif //MODELCOMPONENT_H
