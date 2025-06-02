//
// Created by lukas on 02.06.2025.
//

#include "ButtonSystem.h"
#include "Scene.h" // Pełna definicja klasy Scene

void ButtonSystem::update(GLFWwindow* window) {
    auto buttons = scene->getStorage<UIButtonComponent>();
    if (!buttons) return;

    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    cursorPosition = glm::vec2((float)xpos, (float)ypos);

    for (int i = 0; i < buttons->getQuantity(); i++) {
        auto& button = buttons->components[i];
        button.isHovered = cursorPosition.x >= button.position.x &&
                           cursorPosition.x <= button.position.x + button.size.x &&
                           cursorPosition.y >= button.position.y &&
                           cursorPosition.y <= button.position.y + button.size.y;

        if (button.isHovered && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            button.isPressed = true;
        } else if (button.isPressed && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
            button.isPressed = false;
            if (button.isHovered && button.onClick) {
                button.onClick();
            }
        }
    }
}

void ButtonSystem::makeButton(EntityID id) {
    UIButtonComponent button;
    if (scene->hasComponent<ImageComponent>(id)) {
        auto image = scene->getComponent<ImageComponent>(id);
        auto transfrom = scene->getComponent<Transform>(id);
        button.position = glm::vec2(transfrom.translation.x, transfrom.translation.y);
        button.size = glm::vec2(image.width, image.height);
    }
    button.id = id;
}

void ButtonSystem::setButtonFunction(EntityID id, std::function<void()> onClick) {
    if (scene->hasComponent<UIButtonComponent>(id)) {
        auto& button = scene->getComponent<UIButtonComponent>(id);
        button.onClick = std::move(onClick);
    } else {
        spdlog::error("ButtonSystem: Entity {} does not have UIButtonComponent", id);
    }
}