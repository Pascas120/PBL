#ifndef BUTTONSYSTEM_H
#define BUTTONSYSTEM_H

#include <GLFW/glfw3.h>
#include <functional>
#include <glm/glm.hpp>
#include "ECS/EntityManager.h"
#include "spdlog/spdlog.h"

struct Transform;
struct UIButtonComponent;

class Scene;

class ButtonSystem {
private:
    Scene* scene;
    glm::vec2 cursorPosition;

public:
    explicit ButtonSystem(Scene* scene) : scene(scene) {}

    void update(GLFWwindow* window);
    void makeButton(EntityID id);
    void setButtonFunction(EntityID id, std::function<void()> onClick);
};

#endif // BUTTONSYSTEM_H