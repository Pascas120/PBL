//
// Created by Łukasz Moskwin on 09/04/2025.
//

#ifndef PBL_BUTTONCOMPONENT_H
#define PBL_BUTTONCOMPONENT_H
#include <functional>

class ButtonComponent{
private:
    float x, y, width, height;
public:
    ButtonComponent(float x, float y, float width, float height);
    bool isOver(float mouseX, float mouseY);
    void onClick(std::function<void()> f);
};


#endif //PBL_BUTTONCOMPONENT_H
