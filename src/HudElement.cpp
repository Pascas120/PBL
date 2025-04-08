//
// Created by Łukasz Moskwin on 02/04/2025.
//

#include "HudElement.h"
#include "Transform.h"
#include "stb_image.h"

HudElement::HudElement(){}

HudElement::HudElement(float x, float y, float width, float height)
        : x(x), y(y), width(width), height(height), color(glm::vec4(1.0f)) {

    // VAO/VBO setup
    float vertices[] = {
            // positions    // tex coords
            0.0f, 1.0f,     0.0f, 1.0f,
            1.0f, 0.0f,     1.0f, 0.0f,
            0.0f, 0.0f,     0.0f, 0.0f,

            0.0f, 1.0f,     0.0f, 1.0f,
            1.0f, 1.0f,     1.0f, 1.0f,
            1.0f, 0.0f,     1.0f, 0.0f
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // position
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // tex coords
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    // Texture setup
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}


void HudElement::draw(Shader& shader) {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(x, y, 0));
    model = glm::scale(model, glm::vec3(width, height, 1));
    shader.setMat4("model", model);
    shader.setVec4("color", color);
    shader.setBool("useTexture", hasTexture);

    if (hasTexture) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureId);
        shader.setInt("texture1", 0);
    }

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    // Draw children
    for (int i = 0; i < childCount; i++) {
        if (children[i]) {
            children[i]->draw(shader);
        }
    }
}

void HudElement::setColor(const glm::vec4 &color){
    HudElement::color = color;
}

void HudElement::setTexture(const std::string &texture) {
    HudElement::texture = texture;
    int texWidth, texHeight, nrChannels;
    unsigned char *data = stbi_load(texture.c_str(), &texWidth, &texHeight, &nrChannels, 0);
    hasTexture = data != nullptr;

    if (data) {
        GLenum format = GL_RGB;
        if (nrChannels == 1)
            format = GL_RED;
        else if (nrChannels == 3)
            format = GL_RGB;
        else if (nrChannels == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureId);
        glTexImage2D(GL_TEXTURE_2D, 0, format, texWidth, texHeight, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);
    }
    else {
        printf("Failed to load texture: %s\n", texture.c_str());
    }
}
float HudElement::getX() const{
    return x;
}

float HudElement::getY() const{
    return y;
}

float HudElement::getWidth() const{
    return width;
}

float HudElement::getHeight() const{
    return height;
}

void HudElement::setX(float x){
    HudElement::x = x;
}

void HudElement::setY(float y){
    HudElement::y = y;
}

void HudElement::setWidth(float width){
    HudElement::width = width;
}

void HudElement::setHeight(float height){
    HudElement::height = height;
}

