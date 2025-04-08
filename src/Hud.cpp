//
// Created by Łukasz Moskwin on 02/04/2025.
//

#include "Hud.h"

void Hud::draw(Shader &shader){
    root->draw(shader);
}

void Hud::setRoot(HudElement *root){
    Hud::root = root;
}
