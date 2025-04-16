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

RenderingSystem::RenderingSystem(Scene *scene, Shader &sceneShader, Shader &hudShader): scene(scene), sceneShader(sceneShader), hudShader(hudShader) {}

void RenderingSystem::drawScene(Camera& camera){
    auto models = scene->GetStorage<ModelComponent>();
    auto transforms = scene->GetStorage<Transform>();

    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    sceneShader.setMat4("projection", projection);
    sceneShader.setMat4("view", view);

    for(int i = 0; i < 5000; i++) {
        if (models->has(i)) {
            sceneShader.setMat4("model", transforms->get(i).globalMatrix);
            models->get(i).model->Draw(sceneShader);
        }
    }
}

void RenderingSystem::drawHud() {
    float vertices[] = {
            // Pozycja      // Tekstura
            -0.5f, -0.5f,   0.0f, 0.0f, // Lewy dolny
            0.5f, -0.5f,   1.0f, 0.0f, // Prawy dolny
            0.5f,  0.5f,   1.0f, 1.0f, // Prawy górny
            -0.5f,  0.5f,   0.0f, 1.0f  // Lewy górny
    };

    unsigned int indices[] = {
            0, 1, 2, // Pierwszy trójkąt
            0, 2, 3  // Drugi trójkąt
    };

    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    auto images = scene->GetStorage<ImageComponent>();
    for (int i = 0; i < 5000; i++) {
        if (images->has(i)) {
            auto& image = images->get(i);

            hudShader.use();

            if (!image.texturePath.empty()) {
                // Załaduj teksturę
                int width, height, nrChannels;
                unsigned char* data = stbi_load(image.texturePath.c_str(), &width, &height, &nrChannels, 0);
                if (data) {
                    unsigned int textureID;
                    glGenTextures(1, &textureID);
                    glBindTexture(GL_TEXTURE_2D, textureID);

                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
                    glGenerateMipmap(GL_TEXTURE_2D);

                    stbi_image_free(data);

                    hudShader.setInt("useTexture", true);
                } else {
                    hudShader.setInt("useTexture", false);
                    hudShader.setVec4("color", image.color);
                }
            } else {
                hudShader.setInt("useTexture", false);
                hudShader.setVec4("color", image.color);
            }

            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        }
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void RenderingSystem::draw(Camera& camera){
    drawScene(camera);
    drawHud();
}
