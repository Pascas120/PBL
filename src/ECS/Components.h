//
// Created by Łukasz Moskwin on 14/04/2025.
//

#ifndef PBL_COMPONENTS_H
#define PBL_COMPONENTS_H
#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include <string>

struct Transform {
    glm::vec3 translation = {0.0f, 0.0f, 0.0f};
    glm::quat rotation = {1.0f, 0.0f, 0.0f, 0.0f};
    glm::vec3 scale = {1.0f, 1.0f, 1.0f};
    glm::mat4 localMatrix = glm::mat4(1.0f);
    glm::mat4 globalMatrix = glm::mat4(1.0f);
};

struct ModelComponent {
    Model* model;
};

struct ImageComponent {
    std::string texturePath;
    glm::vec4 color;
};

struct TextComponent {
    std::string font;
    glm::vec4 color;
    std::string text;
};

#endif //PBL_COMPONENTS_H
