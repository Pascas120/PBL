//
// Created by Łukasz Moskwin on 15/04/2025.
//

#include "RenderingSystem.h"
#include "Model.h"
#include "stb_image.h"

//TODO Nie możemy tworzyć tekstur w każdej klatce, trzeba zrobić manager zasobów
constexpr int32_t WINDOW_WIDTH  = 1920;
constexpr int32_t WINDOW_HEIGHT = 1080;

RenderingSystem::RenderingSystem(){

}

RenderingSystem::RenderingSystem(Scene *scene, Shader &sceneShader, Shader &hudShader, Shader &textShader): scene(scene), sceneShader(sceneShader), hudShader(hudShader), textShader(textShader) {}

void RenderingSystem::drawScene(Camera& camera){
    auto models = scene->getStorage<ModelComponent>();
    auto transforms = scene->getStorage<Transform>();

    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = camera.getViewMatrix();
    sceneShader.use();
    sceneShader.setMat4("projection", projection);
    sceneShader.setMat4("view", view);

    for(int i = 0; i <= models->getQuantity(); i++) {
        if (models->has(i)) {
            Transform transform = transforms->get(models->get(i).id);
            sceneShader.setMat4("model", transform.globalMatrix);
            models->get(i).model->draw(sceneShader);
        }
    }
}

void RenderingSystem::drawHud() {
    if (!initializedHud) initHud(); // Inicjalizacja, jeśli nie została wykonana

    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    auto transforms = scene->getStorage<Transform>();
    auto images = scene->getStorage<ImageComponent>();
    auto texts = scene->getStorage<TextComponent>();

    if(images != NULL) {
        hudShader.use();
        for (int i = 0; i < images->getQuantity(); i++) {
                auto& image = images->components[i];

                hudShader.setMat4("model", glm::scale(transforms->get(image.id).globalMatrix, glm::vec3(image.width, image.height, 1.0f)));
                if (!image.texturePath.empty()) {
                    if (GLuint textureID = getTexture(image.texturePath)) {
                        glActiveTexture(GL_TEXTURE0);
                        glBindTexture(GL_TEXTURE_2D, textureID);
                        hudShader.setInt("useTexture", true);
                    } else {
                        hudShader.setInt("useTexture", false);
                        hudShader.setVec4("color", image.color);
                    }
                } else {
                    hudShader.setInt("useTexture", false);
                    hudShader.setVec4("color", image.color);
                }
                glBindVertexArray(hudVAO);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                glBindVertexArray(0);
        }
    }

    if(texts != NULL) {
        for (int i = 0; i < texts->getQuantity(); i++) {
                auto& text = texts->components[i];
                t1.renderText(textShader, text.text, transforms->get(text.id).translation.x, transforms->get(text.id).translation.y, 1.0f, text.color);
        }
    }

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

void RenderingSystem::draw(Camera& camera){
    drawScene(camera);
    drawHud();
}

GLuint RenderingSystem::getTexture(std::string path) {
    if (textures.find(path) != textures.end()) {
        return textures[path];
    }

    int width, height, nrChannels;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
    if (data) {
        GLuint textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(data);

        textures[path] = textureID;
        return textureID;
    } else {
        std::cerr << "Failed to load texture: " << path << std::endl;
        return 0;
    }
}

void RenderingSystem::initHud() {
    if (initializedHud) return;

    float vertices[] = {
        -0.5f, -0.5f, 0.0f, 0.0f,
         0.5f, -0.5f, 1.0f, 0.0f,
         0.5f,  0.5f, 1.0f, 1.0f,
        -0.5f,  0.5f, 0.0f, 1.0f
    };

    unsigned int indices[] = {
        0, 1, 2,
        0, 2, 3
    };

    glGenVertexArrays(1, &hudVAO);
    glGenBuffers(1, &hudVBO);
    glGenBuffers(1, &hudEBO);

    glBindVertexArray(hudVAO);

    glBindBuffer(GL_ARRAY_BUFFER, hudVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, hudEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    t1.init("../../res/fonts/sixtyfour.ttf");

    initializedHud = true;
}