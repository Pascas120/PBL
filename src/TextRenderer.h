//
// Created by Łukasz Moskwin on 08/04/2025.
//

#ifndef PBL_TEXTRENDERER_H
#define PBL_TEXTRENDERER_H
#include <map>
#include <iostream>
#include "ft2build.h"
#include FT_FREETYPE_H
#include "Shader.h"

struct Character {
    unsigned int TextureID; // ID handle of the glyph texture
    glm::ivec2   Size;      // Size of glyph
    glm::ivec2   Bearing;   // Offset from baseline to left/top of glyph
    unsigned int Advance;   // Horizontal offset to advance to next glyph
};

class TextRenderer{
private:
    unsigned int VAO, VBO;
    std::map<GLchar, Character> Characters;
public:
    TextRenderer();
    ~TextRenderer();
    int init(std::string font_name);
    void renderText(Shader *shader, std::string text, float x, float y, float scale, glm::vec3 color);
};


#endif //PBL_TEXTRENDERER_H
