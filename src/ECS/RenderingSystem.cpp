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
#include <unordered_set>

#include "Random.h"


static std::array<glm::vec3, 16> generateSSAONoise() {
    std::array<glm::vec3, 16> noise;
    for (int i = 0; i < 16; ++i) {
        glm::vec3 v = {
            Random::getFloat(-1.0f, 1.0f),
            Random::getFloat(-1.0f, 1.0f),
            0.0f
        };
        noise[i] = glm::normalize(v);
    }

	return noise;
}



RenderingSystem::RenderingSystem(Scene *scene) : scene(scene)
{
	glBindTexture(GL_TEXTURE_2D, shadowFramebuffer.GetDepthTexture());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	GLfloat borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	ssaoNoise = generateSSAONoise();

	glGenTextures(1, &ssaoNoiseTexture);
	glBindTexture(GL_TEXTURE_2D, ssaoNoiseTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT, ssaoNoise.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

RenderingSystem::~RenderingSystem()
{
	glDeleteTextures(1, &ssaoNoiseTexture);
}

void RenderingSystem::drawScene(const Framebuffer& framebuffer, Camera& cameraP1, Camera* cameraP2, const UniformBlockStorage& uniformBlockStorage,
    const std::unordered_map<std::string, Shader*>& postShaders) 
{
    auto models = scene->getStorage<ModelComponent>();
    auto transforms = scene->getStorage<Transform>();
    auto lights = scene->getStorage<DirectionalLightComponent>();
    DirectionalLightComponent mainLight;
    bool useShadows = false;
	if (lights != nullptr && lights->getQuantity() > 0) {
        mainLight = lights->components[0];
        useShadows = true;
    }

    auto [width, height] = framebuffer.GetSizePair();
    if (width == 0 || height == 0) {
        return;
    }

    /*CustomFramebuffer* customFramebufferPtr;
	if (showMotionBlur) {
		customFramebufferPtr = &velFramebuffer;
	}
	else {
		customFramebufferPtr = &normalFramebuffer;
	}
	CustomFramebuffer& customFramebuffer = *customFramebufferPtr;*/
	auto [fboWidth, fboHeight] = customFramebuffer.GetSizePair();
	if (fboWidth != width || fboHeight != height) {
		customFramebuffer.Resize(width, height);
	}
	

	Shader* shadowShader = postShaders.at("ShadowMap");
	

    //##################SHADOW MAP##################

    //CustomFramebuffer shadowFramebuffer = CustomFramebuffer(FramebufferConfig{width, height});
    if(useShadows) {
		glm::mat4 lightProjection = shadowFrustum.getProjectionMatrix();
        glm::mat4 lightView = glm::inverse(transforms->get(mainLight.id).globalMatrix);
        shadowFramebuffer.Bind();
        glClear(GL_DEPTH_BUFFER_BIT);
        shadowShader->use();
        shadowShader->setMat4("lightProjection", lightProjection);
        uniformBlockStorage.cameraBlock.setData("lightProjection", &lightProjection);
        shadowShader->setMat4("lightView", lightView);
        uniformBlockStorage.cameraBlock.setData("lightView", &lightView);

		FrustumPlanes shadowFrustumPlanes = shadowFrustum.getPlanes();
		shadowFrustumPlanes.applyTransform(glm::inverse(lightView));
		std::vector<EntityID> shadowVisibleEntities = getVisibleEntities(shadowFrustumPlanes);

		for (const auto& entityID : shadowVisibleEntities) {
			auto& modelComponent = models->get(entityID);
			auto& transform = transforms->get(entityID);

			shadowShader->setMat4("model", transform.globalMatrix);
			modelComponent.model->draw(shadowShader);
		}


		Shader* ShadowFXAAShader = postShaders.at("ShadowFXAA");
		shadowFxaaFilter(ShadowFXAAShader, shadowFramebuffer, shadowPostFramebuffer);

	}
    //##############################################
    float aspectRatio = (float)width / (float)height;
    auto& frustum = cameraP1.getFrustum();



	//spdlog::info("Rendering {} models", renderingQueueSize);

    auto& cameraBlock = uniformBlockStorage.cameraBlock;
    glm::mat4 viewMatrix = cameraP1.getViewMatrix();
    glm::mat4 projectionMatrix = frustum.getProjectionMatrix();
    glm::mat4 viewProjectionMatrix = projectionMatrix * viewMatrix;

	std::array auxBuffers = { &auxiliaryFramebuffer1, &auxiliaryFramebuffer2,
        &postProcessingFramebuffer1, &postProcessingFramebuffer2, &ssaoFramebuffer };
	for (auto& auxBuffer : auxBuffers) {
		auto [pfboWidth, pfboHeight] = auxBuffer->GetSizePair();
		if (pfboWidth != width || pfboHeight != height) {
            auxBuffer->Resize(width, height);
		}
	}

    CustomFramebuffer* baseOutputFramebuffer;

	if (cameraP2 != nullptr) {

        glEnable(GL_STENCIL_TEST);
        auto stencilShader = postShaders.at("SplitScreenStencil");
        stencilShader->use();
        stencilShader->setBool("swapStencil", false);
        drawSplitScreenStencil();


        drawBase(auxiliaryFramebuffer1, cameraP1, uniformBlockStorage, postShaders, useShadows);

        //draw camera 2 view
        
        stencilShader->use();
        stencilShader->setBool("swapStencil", true);
        drawSplitScreenStencil();

        drawBase(auxiliaryFramebuffer2, *cameraP2, uniformBlockStorage, postShaders, useShadows);


        // draw split screen
        glDisable(GL_STENCIL_TEST);
        glStencilMask(0xFF);
        glStencilFunc(GL_ALWAYS, 0, 0xFF);

		baseOutputFramebuffer = &postProcessingFramebuffer1;
		dynamicSplitScreen(postShaders.at("SplitScreen"), cameraP1, auxiliaryFramebuffer1, auxiliaryFramebuffer2, *baseOutputFramebuffer);
	}
	else {
		baseOutputFramebuffer = &auxiliaryFramebuffer1;
		drawBase(*baseOutputFramebuffer, cameraP1, uniformBlockStorage, postShaders, useShadows);
	}


	//##################POST PROCESSING##################

    
    Shader* FXAAShader = postShaders.at("FXAA");



    

    fxaaFilter(FXAAShader, *baseOutputFramebuffer, *baseOutputFramebuffer, framebuffer);

}

void RenderingSystem::drawHud(const Framebuffer& framebuffer, const std::unordered_map<std::string, Shader*>& postShaders) {
    if (!initializedHud) initHud(); // Inicjalizacja, jeśli nie została wykonana

    auto [width, height] = framebuffer.GetSizePair();
    glm::mat4 ortho = glm::ortho(0.0f, (float)width,  0.0f, (float)height, -1.0f, 1.0f);

    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

    auto transforms = scene->getStorage<Transform>();
    auto images = scene->getStorage<ImageComponent>();
    auto texts = scene->getStorage<TextComponent>();

    framebuffer.Bind();

    static auto lastLogTime = std::chrono::steady_clock::now();

    if (images != nullptr) {
        Shader* hudShader = postShaders.at("HUD");
        for (int i = 0; i < images->getQuantity(); i++) {
            auto& image = images->components[i];

            EntityID entityID = image.id;
            hudShader->use();
            hudShader->setMat4("projection", ortho);

            glm::mat4 modelMatrix = glm::scale(transforms->get(entityID).globalMatrix, glm::vec3(image.width, image.height, 1.0f));
            hudShader->setMat4("model", modelMatrix);


            if (!image.texturePath.empty()) {
                if (GLuint textureID = getTexture(image.texturePath)) {
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, textureID);
                    hudShader->setInt("useTexture", true);
                } else {
                    hudShader->setInt("useTexture", false);
                    hudShader->setVec4("color", image.color);
                }
            } else {
                hudShader->setInt("useTexture", false);
                hudShader->setVec4("color", image.color);
            }
            glBindVertexArray(hudVAO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        }
    }

    if (texts != nullptr) {
        Shader* textShader = postShaders.at("Text");
        for (int i = 0; i < texts->getQuantity(); i++) {
            auto& text = texts->components[i];

            EntityID entityID = text.id;
            textShader->use();
            textShader->setMat4("projection", ortho);
            t1.renderText(textShader, text.text, transforms->get(entityID).translation.x, transforms->get(entityID).translation.y, text.fontSize, text.color);
        }
    }

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
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

    t1.init("../../res/fonts/caramel.ttf");

    initializedHud = true;
}

void RenderingSystem::buildTree() {
    auto models = scene->getStorage<ModelComponent>();
	std::vector<TreeBox<ModelComponent*>> modelComponents;
    modelComponents.reserve(models->getQuantity());

    for (int i = 0; i < models->getQuantity(); i++) {
        models->components[i].transform = &scene->getComponent<Transform>(models->components[i].id);
        if (models->components[i].transform->isStatic)
        {
			TreeBox<ModelComponent*> box;
			box.object = &models->components[i];
			box.globalBox = models->components[i].model->boundingBox.getGlobalBox(*models->components[i].transform);
			modelComponents.push_back(box);
        }
            //modelComponents.push_back(&models->components[i]);
    }

	bvh.build(modelComponents);
    //rootNode = buildBVH(modelComponents);
}

void RenderingSystem::sobelFilter(Shader* sobel, const CustomFramebuffer &in, const Framebuffer &out) {
    out.Bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    sobel->use();
    auto [width, height] = in.GetSizePair();
    sobel->setInt("width", width);
    sobel->setInt("height", height);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, in.GetColorTexture());
    sobel->setInt("textureSampler", 0);

    glBindVertexArray(hudVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void RenderingSystem::motionBlurFilter(Shader* blur, const CustomFramebuffer& in, 
    const CustomFramebuffer& inVel, const Framebuffer& out) {
    out.Bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    blur->use();
    auto [width, height] = in.GetSizePair();
    blur->setInt("width", width);
    blur->setInt("height", height);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, in.GetColorTexture());
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, inVel.GetVelocityTexture());
    blur->setInt("textureSampler", 0);
    blur->setInt("velTextureSampler", 1);

    glBindVertexArray(hudVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void RenderingSystem::fxaaFilter(Shader* fxaa, const CustomFramebuffer& in, 
    const CustomFramebuffer& test, const Framebuffer& out) {
    out.Bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    fxaa->use();
    auto [width, height] = in.GetSizePair();
    fxaa->setVec2("resolution", (float)width, (float)height);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, in.GetColorTexture());
    fxaa->setInt("textureSampler", 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, test.GetColorTexture());
	fxaa->setInt("testSampler", 1);

    glBindVertexArray(hudVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void RenderingSystem::shadowFxaaFilter(Shader* fxaa, const CustomFramebuffer& in, const Framebuffer& out) {
    out.Bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    fxaa->use();
    auto [width, height] = in.GetSizePair();
    fxaa->setVec2("resolution", (float)width, (float)height);
    glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, in.GetDepthTexture());
    fxaa->setInt("shadowMap", 0);

    glBindVertexArray(hudVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void RenderingSystem::ssaoFilter(Shader* ssao, const CustomFramebuffer& gBuffer, const Framebuffer& out) {
    out.Bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	ssao->use();
	auto [width, height] = gBuffer.GetSizePair();
	ssao->setInt("width", width);
	ssao->setInt("height", height);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gBuffer.GetNormalTexture());
	ssao->setInt("normalTexture", 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, ssaoNoiseTexture);
	ssao->setInt("noiseTexture", 2);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, gBuffer.GetDepthTexture());
	ssao->setInt("depthTexture", 3);


	glBindVertexArray(hudVAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void RenderingSystem::ssaoApplyFilter(Shader* ssaoApply, const CustomFramebuffer& in, const CustomFramebuffer& ssao, const Framebuffer& out)
{
	out.Bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	ssaoApply->use();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, in.GetColorTexture());
	ssaoApply->setInt("colorTexture", 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, ssao.GetColorTexture());
	ssaoApply->setInt("ssaoTexture", 1);

	glBindVertexArray(hudVAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void RenderingSystem::dynamicSplitScreen(Shader *dynamicSplitScreen, Camera& camera, const CustomFramebuffer &in,
	CustomFramebuffer &in2, const Framebuffer &out) {
	out.Bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	dynamicSplitScreen->use();

	glm::vec2 viewportSize = glm::vec2(in.GetSizePair().first, in.GetSizePair().second);
	dynamicSplitScreen->setVec2("viewport_size", viewportSize);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, in.GetColorTexture());
	dynamicSplitScreen->setInt("viewport1", 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, in2.GetColorTexture());
	dynamicSplitScreen->setInt("viewport2", 1);
	glBindVertexArray(hudVAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void RenderingSystem::drawSplitScreenStencil()
{
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glStencilMask(0xFF);

    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDepthMask(GL_FALSE);
    glDisable(GL_DEPTH_TEST);

    glBindVertexArray(hudVAO);

    std::array splitScreenBuffers = { &customFramebuffer, &postProcessingFramebuffer1, &postProcessingFramebuffer2, &ssaoFramebuffer,
        &auxiliaryFramebuffer1, &auxiliaryFramebuffer2 };
    for (auto& buffer : splitScreenBuffers)
    {
        buffer->Bind();
        glClear(GL_STENCIL_BUFFER_BIT);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }

    glBindVertexArray(0);

    glStencilMask(0x00);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);

    glStencilFunc(GL_EQUAL, 1, 0xFF);
}

void RenderingSystem::updatePreviousModelMatrices() {
    auto models = scene->getStorage<ModelComponent>();
    for (int i = 0; i < models->getQuantity(); i++) {
        auto& modelComponent = models->components[i];
		auto& transform = scene->getComponent<Transform>(modelComponent.id);
        modelComponent.prevModelMatrix = transform.globalMatrix;
    }
}

std::vector<EntityID> RenderingSystem::getVisibleEntities(const FrustumPlanes& frustumPlanes)
{
	std::vector<EntityID> visibleEntities;

	auto models = scene->getStorage<ModelComponent>();
	auto transforms = scene->getStorage<Transform>();

    auto rootNode = bvh.getRoot();
    if (useTree && rootNode) {
        /*spdlog::info("planes: {}, {}, {}",
                     frustum.getPlanes().nearFace.normal.x,frustum.getPlanes().nearFace.normal.y,frustum.getPlanes().nearFace.normal.z);
        */
        traverseBVHFrustum(rootNode, frustumPlanes, visibleEntities);
        //spdlog::info("Tree: {} entities visible", visibleEntities.size());
    }
    else if (useTree && !rootNode) {
        spdlog::warn("BVH root node is null, skipping frustum culling.");
    }

    for (int i = 0; i < models->getQuantity(); i++) {
        auto& modelComponent = models->components[i];
        auto& transform = transforms->get(modelComponent.id);
        if (!useTree || !transform.isStatic)
        {
            auto& boundingBox = modelComponent.model->boundingBox;

            if (isOnFrustum(boundingBox, frustumPlanes, transform.globalMatrix)) {
                visibleEntities.push_back(modelComponent.id);
            }
        }
    }

	return visibleEntities;
}

void RenderingSystem::drawBase(const CustomFramebuffer& outputFramebuffer, Camera& camera, const UniformBlockStorage& uniformBlockStorage,
    const std::unordered_map<std::string, Shader*>& postShaders, bool useShadows) {
	auto& frustum = camera.getFrustum();
    auto models = scene->getStorage<ModelComponent>();
    auto transforms = scene->getStorage<Transform>();

    FrustumPlanes globalPlanes = frustum.getPlanes();
    globalPlanes.applyTransform(camera.getInvViewMatrix());

	std::vector<EntityID> renderingQueue = getVisibleEntities(globalPlanes);


    Shader* shadowShader = postShaders.at("ShadowMap");

	//spdlog::info("Rendering {} models", renderingQueue.size());

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
    GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
    glDrawBuffers(4, drawBuffers);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const float clearVec4[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    glClearBufferfv(GL_COLOR, 1, clearVec4);
    glClearBufferfv(GL_COLOR, 2, clearVec4);

    const float clearVec2[] = { 0.0f, 0.0f };
    glClearBufferfv(GL_COLOR, 3, clearVec2);

	std::unordered_set<Shader*> shadersUpdatedWithShadowMap;

    if (useShadows)
    {
        glActiveTexture(GL_TEXTURE0 + 1);
        glBindTexture(GL_TEXTURE_2D, shadowPostFramebuffer.GetColorTexture());
    }

	for (const auto& entityID : renderingQueue) {
		auto& modelComponent = models->get(entityID);

		auto& info = scene->getComponent<ObjectInfoComponent>(entityID);

		glm::mat4 modelMatrix = transforms->get(entityID).globalMatrix;
        modelComponent.shader->use();
		if (useShadows && shadersUpdatedWithShadowMap.find(modelComponent.shader) == shadersUpdatedWithShadowMap.end()) {
			modelComponent.shader->setInt("shadowMap", 1);
			shadersUpdatedWithShadowMap.insert(modelComponent.shader);
		}

		if (scene->hasComponent<AnimationComponent>(entityID)) {
			auto& animationComponent = scene->getComponent<AnimationComponent>(entityID);
			auto boneMatrices = animationComponent.animator->GetFinalBoneMatrices();
			for (int i = 0; i < boneMatrices.size(); ++i) {
				//
				// for (int row = 0; row < 4; ++row)
				// {
				// 	if(boneMatrices[i] == glm::mat4(1.0f)) {
				// 		continue;
				// 	}
				//     spdlog::info("[{}, {}, {}, {}]",
				//                  boneMatrices[i][row][0],
				//                  boneMatrices[i][row][1],
				//                  boneMatrices[i][row][2],
				//                  boneMatrices[i][row][3]);
				// }
				modelComponent.shader->setMat4("finalBonesMatrices[" + std::to_string(i) + "]", boneMatrices[i]);
			}
		}

		if (showMotionBlur)
		{
			modelComponent.shader->setMat4("prevModel", modelComponent.prevModelMatrix);
		}
		else
		{
			modelComponent.shader->setMat4("prevModel", modelMatrix);
		}

        modelComponent.shader->setMat4("model", modelMatrix);
		modelComponent.model->draw(modelComponent.shader, modelComponent.color);
    }

    Shader* sobelShader = postShaders.at("Sobel");
    Shader* motionBlurShader = postShaders.at("MotionBlur");
    Shader* ssaoShader = postShaders.at("SSAO");
    Shader* ssaoApplyShader = postShaders.at("SSAOApply");

    ssaoFilter(ssaoShader, customFramebuffer, ssaoFramebuffer);
    sobelFilter(sobelShader, customFramebuffer, postProcessingFramebuffer1);
    ssaoApplyFilter(ssaoApplyShader, postProcessingFramebuffer1, ssaoFramebuffer, 
		outputFramebuffer);


    /*if (showMotionBlur)
    {
        cameraBlock.setData("prevViewProjection", &viewProjectionMatrix);
        motionBlurFilter(motionBlurShader, postProcessingFramebuffer2, customFramebuffer, outputFramebuffer);
    }*/
}