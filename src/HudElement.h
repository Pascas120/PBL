//
// Created by Łukasz Moskwin on 02/04/2025.
//

#ifndef PBL_HUDELEMENT_H
#define PBL_HUDELEMENT_H

#include "Mesh.h"
#include "GameObject.h"
#include <ft2build.h>
#include FT_FREETYPE_H

class HudElement: public GameObject{
private:
    float x, y, width, height;
    glm::vec4 color;
    std::string texture;
    GLuint textureId;
    GLuint VAO, VBO;
    bool hasTexture = false;

public:
    HudElement();
    HudElement(float x, float y, float width, float height);

    void setColor(const glm::vec4 &color);

    void draw(Shader& shader) override;

    void setTexture(const std::string &texture);

    float getX() const;

    float getY() const;

    float getWidth() const;

    float getHeight() const;

    void setX(float x);

    void setY(float y);

    void setWidth(float width);

    void setHeight(float height);
};


#endif //PBL_HUDELEMENT_H
