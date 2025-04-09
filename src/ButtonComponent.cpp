//
// Created by Łukasz Moskwin on 09/04/2025.
//

#include "ButtonComponent.h"

ButtonComponent::ButtonComponent(float x, float y, float width, float height) : x(x), y(y), width(width),
                                                                                height(height){}
bool ButtonComponent::isOver(float mouseX, float mouseY) {
    return mouseX > x && mouseX < x + width && mouseY > y && mouseY < y + height;
}

void ButtonComponent::onClick(std::function<void()> f){
    f();
}