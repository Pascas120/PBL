//
// Created by Łukasz Moskwin on 02/04/2025.
//

#ifndef PBL_HUD_H
#define PBL_HUD_H


#include "HudElement.h"

class Hud{
private:
    HudElement* root;
public:
    void setRoot(HudElement *root);

    void draw(Shader &shader);
};


#endif //PBL_HUD_H
