//
// Created by Łukasz Moskwin on 15/04/2025.
//

#include "RenderingSystem.h"
#include "Model.h"
#include "stb_image.h"
#include "glm/gtc/matrix_transform.hpp"
#include "Scene.h"
#include "spdlog/spdlog.h"
#include "glm/gtc/type_ptr.hpp"



RenderingSystem::RenderingSystem(Scene *scene) : scene(scene) {}

void RenderingSystem::drawScene(const Framebuffer& framebuffer, Camera& camera, const UniformBlockStorage& uniformBlockStorage) {
    auto models = scene->getStorage<ModelComponent>();
    auto transforms = scene->getStorage<Transform>();
    auto lights = scene->getStorage<DirectionalLightComponent>();
    DirectionalLightComponent mainLight;
    bool useShadows = false;
	if (lights != nullptr && lights->getQuantity() > 0) {
        mainLight = lights->components[0];
        useShadows = true;
    }
    uint16_t renderingQueueSize = 0;

    EntityID renderingQueue[MAX_ENTITIES];

    auto [width, height] = framebuffer.GetSizePair();
    if (width == 0 || height == 0) {
        return;
    }

	std::vector<AttachmentType> attachments = { AttachmentType::COLOR, AttachmentType::DEPTH };
	if (showMotionBlur) {
		attachments.push_back(AttachmentType::VELOCITY);
	}
    CustomFramebuffer* customFramebufferPtr;
	if (showMotionBlur) {
		customFramebufferPtr = &velFramebuffer;
	}
	else {
		customFramebufferPtr = &normalFramebuffer;
	}
	CustomFramebuffer& customFramebuffer = *customFramebufferPtr;
	auto [fboWidth, fboHeight] = customFramebuffer.GetSizePair();
	if (fboWidth != width || fboHeight != height) {
		customFramebuffer.Resize(width, height);
		postProcessingFramebuffer.Resize(width, height);
	}
    //##################SHADOW MAP##################
    Shader* shadowShader = postShaders["ShadowMap"];
    //CustomFramebuffer shadowFramebuffer = CustomFramebuffer(FramebufferConfig{width, height});
    /*if(useShadows) {
        glm::vec3 lightPos = transforms->get(mainLight.id).translation;
        glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 1.0f, 20.0f);
        glm::mat4 lightView = glm::inverse(transforms->get(mainLight.id).globalMatrix);
        shadowFramebuffer.Bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shadowShader->use();
        shadowShader->setMat4("lightProjection", lightProjection);
        uniformBlockStorage.cameraBlock.setData("lightProjection", &lightProjection);
        shadowShader->setMat4("lightView", lightView);
        uniformBlockStorage.cameraBlock.setData("lightView", &lightView);
        shadowShader->setVec3("lightPos", lightPos);

    }*/
    //##############################################
    float aspectRatio = (float)width / (float)height;
    auto& frustum = camera.getFrustum();

    FrustumPlanes globalPlanes = frustum.getPlanes();
    globalPlanes.applyTransform(camera.getInvViewMatrix());

    //buildTree();
    std::vector<EntityID> visibleEntities;

    visibleEntities.reserve(models->getQuantity());
    if (useTree && rootNode) {
        /*spdlog::info("planes: {}, {}, {}",
                     frustum.getPlanes().nearFace.normal.x,frustum.getPlanes().nearFace.normal.y,frustum.getPlanes().nearFace.normal.z);
        */
        traverseBVHFrustum(rootNode.get(), globalPlanes, visibleEntities);
		spdlog::info("Tree: {} entities visible", visibleEntities.size());
    } else if (!rootNode) {
        spdlog::warn("BVH root node is null, skipping frustum culling.");
    }

    for (int i = 0; i < models->getQuantity(); i++) {
		if (!useTree || !transforms->get(models->components[i].id).isStatic)
        {
            auto& modelComponent = models->components[i];
            auto& boundingBox = modelComponent.model->boundingBox;

		    /*if(useShadows) {
		        shadowShader->setMat4("model", transforms->get(modelComponent.id).globalMatrix);
		        modelComponent.model->draw(postShaders["ShadowMap"]);
		    }*/
            if (isOnFrustum(boundingBox, globalPlanes, transforms->get(modelComponent.id))) {
                renderingQueue[renderingQueueSize++] = modelComponent.id;
            }
        }
    }

    for(int i = 0; i < visibleEntities.size(); i++) {
        EntityID entityID = visibleEntities[i];
        if (models->has(entityID)) {
                renderingQueue[renderingQueueSize++] = entityID;
        }
    }

	spdlog::info("Rendering {} models", renderingQueueSize);

    auto& cameraBlock = uniformBlockStorage.cameraBlock;
    glm::mat4 viewMatrix = camera.getViewMatrix();
    glm::mat4 invViewMatrix = camera.getInvViewMatrix();
    glm::vec3 cameraPosition = invViewMatrix[3];
    glm::mat4 projectionMatrix = frustum.getProjectionMatrix();
    glm::mat4 invProjectionMatrix = glm::inverse(projectionMatrix);
    glm::mat4 viewProjectionMatrix = projectionMatrix * viewMatrix;
    glm::mat4 invViewProjectionMatrix = glm::inverse(viewProjectionMatrix);

    cameraBlock.setData("viewPos", &cameraPosition);
    cameraBlock.setData("view", &viewMatrix);
    cameraBlock.setData("invView", &invViewMatrix);
    cameraBlock.setData("projection", &projectionMatrix);
    cameraBlock.setData("invProjection", &invProjectionMatrix);
    cameraBlock.setData("viewProjection", &viewProjectionMatrix);
    cameraBlock.setData("invViewProjection", &invViewProjectionMatrix);

    customFramebuffer.Bind();
    GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, drawBuffers);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    /*if(useShadows) {
        models->get(renderingQueue[0]).shader->use();
        shadowFramebuffer.Bind();
        glActiveTexture(GL_TEXTURE0+1);
        glBindTexture(GL_TEXTURE_2D, shadowFramebuffer.GetDepthTexture());
        models->get(renderingQueue[0]).shader->setInt("shadowMap", 1);
    }*/

    for (int i = 0; i < renderingQueueSize; i++) {
        auto& modelComponent = models->get(renderingQueue[i]);

        EntityID entityID = modelComponent.id;

		glm::mat4 modelMatrix = transforms->get(entityID).globalMatrix;
        modelComponent.shader->use();
		if (showMotionBlur)
		{
			modelComponent.shader->setMat4("prevModel", modelComponent.prevModelMatrix);
		}
		else
		{
			modelComponent.shader->setMat4("prevModel", modelMatrix);
		}

        modelComponent.shader->setMat4("model", modelMatrix);
        modelComponent.model->draw(modelComponent.shader);
    }


    if (showMotionBlur)
    {
        sobelFilter(customFramebuffer, postProcessingFramebuffer);

		cameraBlock.setData("prevViewProjection", &viewProjectionMatrix);
		motionBlurFilter(postProcessingFramebuffer, customFramebuffer, framebuffer);
    }
    else
    {
		sobelFilter(customFramebuffer, framebuffer);
    }

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
        -1.0f, -1.0f, 0.0f, 0.0f,
         1.0f, -1.0f, 1.0f, 0.0f,
         1.0f,  1.0f, 1.0f, 1.0f,
        -1.0f,  1.0f, 0.0f, 1.0f
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

void RenderingSystem::buildTree() {
    auto models = scene->getStorage<ModelComponent>();
    std::vector<ModelComponent*> modelComponents;
    modelComponents.reserve(models->getQuantity());

    for (int i = 0; i < models->getQuantity(); i++) {
        models->components[i].transform = &scene->getComponent<Transform>(models->components[i].id);
        if (models->components[i].transform->isStatic)
            modelComponents.push_back(&models->components[i]);
    }

    rootNode = buildBVH(modelComponents);
}

void RenderingSystem::addPostShader(const std::string &name, Shader* shader) {
    postShaders[name] = shader;
}

void RenderingSystem::sobelFilter(const CustomFramebuffer &in, const Framebuffer &out) {
    out.Bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    Shader* shader = postShaders["Sobel"];
    shader->use();
    auto [width, height] = in.GetSizePair();
    shader->setInt("width", width);
    shader->setInt("height", height);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, in.GetColorTexture());
    shader->setInt("textureSampler", 0);

    glBindVertexArray(hudVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void RenderingSystem::motionBlurFilter(const CustomFramebuffer& in, const CustomFramebuffer& inVel, const Framebuffer& out) {
    out.Bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	Shader* shader = postShaders["MotionBlur"];
    shader->use();
    auto [width, height] = in.GetSizePair();
    shader->setInt("width", width);
    shader->setInt("height", height);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, in.GetColorTexture());
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, inVel.GetVelocityTexture());
    shader->setInt("textureSampler", 0);
	shader->setInt("velTextureSampler", 1);

    glBindVertexArray(hudVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void RenderingSystem::updatePreviousModelMatrices() {
    auto models = scene->getStorage<ModelComponent>();
    for (int i = 0; i < models->getQuantity(); i++) {
        auto& modelComponent = models->components[i];
		auto& transform = scene->getComponent<Transform>(modelComponent.id);
        modelComponent.prevModelMatrix = transform.globalMatrix;
    }
}