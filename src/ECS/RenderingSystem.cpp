//
// Created by Łukasz Moskwin on 15/04/2025.
//

#include "RenderingSystem.h"
#include "Model.h"
#include "stb_image.h"
#include "glm/gtc/matrix_transform.hpp"
#include "Scene.h"
#include "spdlog/spdlog.h"

RenderingSystem::RenderingSystem(Scene *scene) : scene(scene) {}

void RenderingSystem::drawScene(const Framebuffer& framebuffer, Camera& camera) {
    auto models = scene->getStorage<ModelComponent>();
    auto transforms = scene->getStorage<Transform>();

    uint16_t renderingQueueSize = 0;

    ModelComponent* renderingQueue = new ModelComponent[MAX_ENTITIES];

    auto [width, height] = framebuffer.GetSizePair();
	if (width == 0 || height == 0) {
		return;
	}
    float aspectRatio = (float)width / (float)height;
    camera.createFrustum(aspectRatio);
    for (int i = 0; i < models->getQuantity(); i++) {
        auto& modelComponent = models->components[i];
        AABBBV& boundingBox = modelComponent.model->boundingBox;

        if (boundingBox.isOnFrustum(camera.frustum, transforms->get(modelComponent.id))) {
            renderingQueue[renderingQueueSize++] = modelComponent;
        }
    }

    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)width / (float)height, 0.1f, 100.0f);
    glm::mat4 view = camera.getViewMatrix();

	framebuffer.Bind();
    for (int i = 0; i < renderingQueueSize; i++) {
        auto& modelComponent = renderingQueue[i];

        EntityID entityID = modelComponent.id;


        modelComponent.shader->use();

        // TODO: uniform blocks
        modelComponent.shader->setMat4("projection", projection);
        modelComponent.shader->setMat4("view", view);


        modelComponent.shader->setMat4("model", transforms->get(entityID).globalMatrix);
        modelComponent.model->draw(modelComponent.shader);
    }

	delete[] renderingQueue;
}

void RenderingSystem::drawHud(const Framebuffer& framebuffer) {
    if (!initializedHud) initHud(); // Inicjalizacja, jeśli nie została wykonana

    auto [width, height] = framebuffer.GetSizePair();
    glm::mat4 ortho = glm::ortho(0.0f, (float)width, (float)height, 0.0f, -1.0f, 1.0f);


    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    auto transforms = scene->getStorage<Transform>();
    auto images = scene->getStorage<ImageComponent>();
    auto texts = scene->getStorage<TextComponent>();

    if (images != NULL) {
        for (int i = 0; i < images->getQuantity(); i++) {
            auto& image = images->components[i];

            EntityID entityID = image.id;
            image.shader->use();

            // TODO: uniform blocks
            image.shader->setMat4("projection", ortho);

            image.shader->setMat4("model", glm::scale(transforms->get(entityID).globalMatrix, glm::vec3(image.width, image.height, 1.0f)));
            if (!image.texturePath.empty()) {
                if (GLuint textureID = getTexture(image.texturePath)) {
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, textureID);
                    image.shader->setInt("useTexture", true);
                } else {
                    image.shader->setInt("useTexture", false);
                    image.shader->setVec4("color", image.color);
                }
            } else {
                image.shader->setInt("useTexture", false);
                image.shader->setVec4("color", image.color);
            }
            glBindVertexArray(hudVAO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        }
    }

    if (texts != NULL) {
        for (int i = 0; i < texts->getQuantity(); i++) {
            auto& text = texts->components[i];

            EntityID entityID = text.id;

            text.shader->use();
            text.shader->setMat4("projection", ortho);
            t1.renderText(text.shader, text.text, transforms->get(entityID).translation.x, transforms->get(entityID).translation.y, 1.0f, text.color);
        }
    }

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
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