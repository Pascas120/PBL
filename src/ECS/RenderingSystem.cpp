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


static std::array<glm::vec3, 64> generateSSAOKernel() {
    std::array<glm::vec3, 64> kernel;
    for (int i = 0; i < 64; ++i) {
        glm::vec3 sample = Random::inUnitSphere();
        sample.z = std::abs(sample.z);
        sample *= Random::getFloat(0.0f, 1.0f);

        float scale = float(i) / 64.0f;
        scale = glm::mix(0.1f, 1.0f, scale * scale);
        sample *= scale;

        kernel[i] = sample;
    }
    return kernel;
}

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

	ssaoKernel = generateSSAOKernel();
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
    uint16_t renderingQueueSize = 0;

    EntityID renderingQueue[MAX_ENTITIES];

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
        glm::vec3 lightPos = transforms->get(mainLight.id).translation;
        glm::mat4 lightProjection = glm::ortho(-30.0f, 30.0f, -30.0f, 30.0f, 1.0f, 60.0f);
        glm::mat4 lightView = glm::inverse(transforms->get(mainLight.id).globalMatrix);
        shadowFramebuffer.Bind();
        glClear(GL_DEPTH_BUFFER_BIT);
        shadowShader->use();
        shadowShader->setMat4("lightProjection", lightProjection);
        uniformBlockStorage.cameraBlock.setData("lightProjection", &lightProjection);
        shadowShader->setMat4("lightView", lightView);
        uniformBlockStorage.cameraBlock.setData("lightView", &lightView);
        shadowShader->setVec3("lightPos", lightPos);

    }

	if (useShadows) {
		Shader* ShadowFXAAShader = postShaders.at("ShadowFXAA");
		shadowFxaaFilter(ShadowFXAAShader, shadowFramebuffer, shadowPostFramebuffer);


		glActiveTexture(GL_TEXTURE0 + 1);
		glBindTexture(GL_TEXTURE_2D, shadowPostFramebuffer.GetColorTexture());
	}
    //##############################################
    float aspectRatio = (float)width / (float)height;
    auto& frustum = cameraP1.getFrustum();

    FrustumPlanes globalPlanes = frustum.getPlanes();
    globalPlanes.applyTransform(cameraP1.getInvViewMatrix());

    //buildTree();
    std::vector<EntityID> visibleEntities;

    visibleEntities.reserve(models->getQuantity());
    if (useTree && rootNode) {
        /*spdlog::info("planes: {}, {}, {}",
                     frustum.getPlanes().nearFace.normal.x,frustum.getPlanes().nearFace.normal.y,frustum.getPlanes().nearFace.normal.z);
        */
        traverseBVHFrustum(rootNode.get(), globalPlanes, visibleEntities);
		//spdlog::info("Tree: {} entities visible", visibleEntities.size());
    } else if (useTree && !rootNode) {
        spdlog::warn("BVH root node is null, skipping frustum culling.");
    }

    for (int i = 0; i < models->getQuantity(); i++) {
        auto& modelComponent = models->components[i];

        if (useShadows) {
            shadowShader->setMat4("model", transforms->get(modelComponent.id).globalMatrix);
            modelComponent.model->draw(shadowShader);
        }
		if (!useTree || !transforms->get(models->components[i].id).isStatic)
        {
            auto& boundingBox = modelComponent.model->boundingBox;

		    
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

	//spdlog::info("Rendering {} models", renderingQueueSize);

    auto& cameraBlock = uniformBlockStorage.cameraBlock;
    glm::mat4 viewMatrix = cameraP1.getViewMatrix();
    glm::mat4 projectionMatrix = frustum.getProjectionMatrix();
    glm::mat4 viewProjectionMatrix = projectionMatrix * viewMatrix;

	std::array postBuffers = { &postProcessingFramebuffer1, &postProcessingFramebuffer2, &ssaoFramebuffer };
	for (auto& postBuffer : postBuffers) {
		auto [pfboWidth, pfboHeight] = postBuffer->GetSizePair();
		if (pfboWidth != width || pfboHeight != height) {
			postBuffer->Resize(width, height);
		}
	}

	if(cameraP2 != nullptr) {
		drawBase(postProcessingFramebuffer1, cameraP1, uniformBlockStorage, renderingQueue, renderingQueueSize, useShadows);
		glm::mat4 viewMatrixP2 = cameraP2->getViewMatrix();
		glm::mat4 projectionMatrixP2 = cameraP2->getFrustum().getProjectionMatrix();
		glm::mat4 viewProjectionMatrixP2 = projectionMatrixP2 * viewMatrixP2;

		drawBase(postProcessingFramebuffer2, *cameraP2, uniformBlockStorage, renderingQueue, renderingQueueSize, useShadows);

		dynamicSplitScreen(postShaders.at("SplitScreen"), cameraP1, postProcessingFramebuffer1, postProcessingFramebuffer2, customFramebuffer);
	}
	else {
		drawBase(customFramebuffer, cameraP1, uniformBlockStorage, renderingQueue, renderingQueueSize, useShadows);
	}


	//##################POST PROCESSING##################

    Shader* sobelShader = postShaders.at("Sobel");
    Shader* motionBlurShader = postShaders.at("MotionBlur");
    Shader* FXAAShader = postShaders.at("FXAA");
	Shader* ssaoShader = postShaders.at("SSAO");
	Shader* ssaoApplyShader = postShaders.at("SSAOApply");



    ssaoFilter(ssaoShader, customFramebuffer, ssaoFramebuffer);
    sobelFilter(sobelShader, customFramebuffer, postProcessingFramebuffer1);
	ssaoApplyFilter(ssaoApplyShader, postProcessingFramebuffer1, ssaoFramebuffer, postProcessingFramebuffer2);

    fxaaFilter(FXAAShader, postProcessingFramebuffer2, postProcessingFramebuffer2,
        showMotionBlur ? postProcessingFramebuffer1 : framebuffer);

    if (showMotionBlur)
    {
        cameraBlock.setData("prevViewProjection", &viewProjectionMatrix);
		motionBlurFilter(motionBlurShader, postProcessingFramebuffer1, customFramebuffer, framebuffer);
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

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gBuffer.GetPositionTexture());
	ssao->setInt("positionTexture", 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gBuffer.GetNormalTexture());
	ssao->setInt("normalTexture", 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, ssaoNoiseTexture);
	ssao->setInt("noiseTexture", 2);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, gBuffer.GetDepthTexture());
	ssao->setInt("depthTexture", 3);

	for (int i = 0; i < ssaoKernel.size(); i++) {
		ssao->setVec3("samples[" + std::to_string(i) + "]", ssaoKernel[i]);
	}

	glBindVertexArray(hudVAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void RenderingSystem::ssaoApplyFilter(Shader* ssaoApply, const CustomFramebuffer& in, const CustomFramebuffer& ssao, const Framebuffer& out)
{
	out.Bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	ssaoApply->use();
	auto [width, height] = in.GetSizePair();
	ssaoApply->setInt("width", width);
	ssaoApply->setInt("height", height);

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
	glm::vec3 p1 = scene->getComponent<Transform>(scene->getComponentArray<ButterController>()[0].id).translation;
	glm::vec3 p2 = scene->getComponent<Transform>(scene->getComponentArray<BreadController>()[0].id).translation;

	// Próg aktywacji podziału
	float split_threshold = 3.0f;
	bool split_active = glm::distance(p1, p2) > split_threshold;

	// Oblicz kierunek między graczami w świecie
	glm::vec3 split_dir_world = glm::normalize(p2 - p1);

	glm::mat4 projection = camera.getFrustum().getProjectionMatrix();
	glm::mat4 view = camera.getViewMatrix();

	// Rzutuj pozycje graczy na ekran
	glm::vec4 clip = projection * view * glm::vec4(p1, 1.0f);
	glm::vec3 ndc = glm::vec3(clip) / clip.w;
	glm::vec2 screen_p1 = glm::vec2(ndc.x * 0.5f + 0.5f, ndc.y * 0.5f + 0.5f);

	clip = projection * view * glm::vec4(p2, 1.0f);
	ndc = glm::vec3(clip) / clip.w;
	glm::vec2 screen_p2 = glm::vec2(ndc.x * 0.5f + 0.5f, ndc.y * 0.5f + 0.5f);

	glm::vec2 viewportSize = glm::vec2(in.GetSizePair().first, in.GetSizePair().second);

	dynamicSplitScreen->setBool("split_active", split_active);
	dynamicSplitScreen->setVec2("player1_screen_pos", screen_p1);
	dynamicSplitScreen->setVec2("player2_screen_pos", screen_p2);
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

void RenderingSystem::updatePreviousModelMatrices() {
    auto models = scene->getStorage<ModelComponent>();
    for (int i = 0; i < models->getQuantity(); i++) {
        auto& modelComponent = models->components[i];
		auto& transform = scene->getComponent<Transform>(modelComponent.id);
        modelComponent.prevModelMatrix = transform.globalMatrix;
    }
}

void RenderingSystem::drawBase(const CustomFramebuffer& customFramebuffer, Camera& camera, const UniformBlockStorage& uniformBlockStorage, EntityID* renderingQueue, uint16_t renderingQueueSize, bool useShadows) {
	auto& frustum = camera.getFrustum();
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
	GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, drawBuffers);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	static const float zeroVelocity[] = { 0, 0 };
	glClearBufferfv(GL_COLOR, 2, zeroVelocity);

	std::unordered_set<Shader*> shadersUpdatedWithShadowMap;

	auto models = scene->getStorage<ModelComponent>();
	auto transforms = scene->getStorage<Transform>();

    for (int i = 0; i < renderingQueueSize; i++) {
        auto& modelComponent = models->get(renderingQueue[i]);

        EntityID entityID = modelComponent.id;

		glm::mat4 modelMatrix = transforms->get(entityID).globalMatrix;
        modelComponent.shader->use();
		if (useShadows && shadersUpdatedWithShadowMap.find(modelComponent.shader) == shadersUpdatedWithShadowMap.end()) {
			modelComponent.shader->setInt("shadowMap", 1);
			shadersUpdatedWithShadowMap.insert(modelComponent.shader);
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
}