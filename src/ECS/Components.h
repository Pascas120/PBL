//
// Created by Łukasz Moskwin on 14/04/2025.
//

#ifndef PBL_COMPONENTS_H
#define PBL_COMPONENTS_H
#include "glm/glm.hpp"
#include <string>
#include "ComponentManager.h"

struct Transform {
    glm::vec3 translate{0.0f, 0.0f, 0.0f};
    glm::vec3 rotate{0.0f, 0.0f, 0.0f};
    glm::vec3 scale{1.0f, 1.0f, 1.0f};

    glm::mat4 globalMatrix{1.0f};
    glm::mat4 localMatrix{1.0f};

    bool dirty{false};
    Entity parentId{0};
};

struct ModelComponent {
    Entity id;
    uint16_t modelId;
};

struct ImageComponent {
    Entity id;
    uint16_t textureId;
    glm::vec4 color;
};

struct TextComponent {
    uint16_t id;
    std::string font;
    glm::vec4 color;
    std::string text;
};

#endif //PBL_COMPONENTS_H
