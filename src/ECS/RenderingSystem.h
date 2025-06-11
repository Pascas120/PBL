//
// Created by Łukasz Moskwin on 15/04/2025.
//

#ifndef PBL_RENDERINGSYSTEM_H
#define PBL_RENDERINGSYSTEM_H


#include <map>
#include <array>

#include "ComponentStorage.h"
#include "Components.h"
#include "KDTree.h"

class Scene;
#include "Camera.h"
#include "Shader.h"
#include "TextRenderer.h"
#include "Framebuffer.h"
#include "UniformBuffer.h"

bool isOnFrustum(const BoundingBox& aabb, const FrustumPlanes& camFrustum, const Transform& transform);

class RenderingSystem {
private:
    Scene* scene;
    //Shader sceneShader;
    //Shader hudShader;
    //Shader textShader;


    std::map<std::string, GLuint> textures;
    GLuint getTexture(std::string path);
    GLuint hudVAO, hudVBO, hudEBO;
    bool initializedHud = false;
    void initHud();
    TextRenderer t1;
    std::unique_ptr<BVHNode> rootNode;
    
    std::array<glm::vec3, 16> ssaoNoise;
	GLuint ssaoNoiseTexture;

    void sobelFilter(Shader* sobel, const CustomFramebuffer& in, const Framebuffer& out);
    void motionBlurFilter(Shader* blur, const CustomFramebuffer& in,
        const CustomFramebuffer& inVel, const Framebuffer& out);
    void fxaaFilter(Shader* fxaa, const CustomFramebuffer& in, const CustomFramebuffer& test, const Framebuffer& out);
    void shadowFxaaFilter(Shader* fxaa, const CustomFramebuffer& in, const Framebuffer& out);
	void ssaoFilter(Shader* ssao, const CustomFramebuffer& gBuffer, const Framebuffer& out);
	void ssaoApplyFilter(Shader* ssaoApply, const CustomFramebuffer& in, const CustomFramebuffer& ssao, const Framebuffer& out);
	void dynamicSplitScreen(Shader* dynamicSplitScreen,  Camera& camera, const CustomFramebuffer& in, CustomFramebuffer& in2, const Framebuffer& out);

	void drawBase(const CustomFramebuffer& outputFramebuffer, Camera& camera, const UniformBlockStorage& uniformBlockStorage, 
        const std::unordered_map<std::string, Shader*>& postShaders, bool useShadows);


	CustomFramebuffer customFramebuffer{ FramebufferConfig{ 1920, 1080, 
		{ AttachmentType::COLOR, AttachmentType::DEPTH, AttachmentType::POSITION, 
        AttachmentType::NORMAL, AttachmentType::VELOCITY } } };

    CustomFramebuffer auxiliaryFramebuffer1{ FramebufferConfig{ 1920, 1080, { AttachmentType::COLOR } } };
	CustomFramebuffer auxiliaryFramebuffer2{ FramebufferConfig{ 1920, 1080, { AttachmentType::COLOR } } };
	CustomFramebuffer postProcessingFramebuffer1{ FramebufferConfig{ 1920, 1080, { AttachmentType::COLOR } } };
    CustomFramebuffer postProcessingFramebuffer2{ FramebufferConfig{ 1920, 1080, { AttachmentType::COLOR } } };
	CustomFramebuffer ssaoFramebuffer{ FramebufferConfig{ 1920, 1080, { AttachmentType::COLOR }, GL_RED, GL_FLOAT } };

    const uint32_t shadowMapWidth = 2048;
    const uint32_t shadowMapHeight = 2048;
    CustomFramebuffer shadowFramebuffer = CustomFramebuffer(FramebufferConfig{ shadowMapWidth, shadowMapHeight, { AttachmentType::DEPTH } });
	CustomFramebuffer shadowPostFramebuffer = CustomFramebuffer(FramebufferConfig{ shadowMapWidth, shadowMapHeight, { AttachmentType::COLOR } });

public:
    //RenderingSystem(Scene* scene, Shader &sceneShader, Shader &hudShader, Shader &textShader);
    RenderingSystem(Scene* scene);
	~RenderingSystem();
	void drawScene(const Framebuffer& framebuffer, Camera& cameraP1, Camera* cameraP2, const UniformBlockStorage& uniformBlockStorage,
	const std::unordered_map<std::string, Shader*>& postShaders);
    void drawHud(const Framebuffer& framebuffer);
    void buildTree();

    void updatePreviousModelMatrices();

	bool useTree = true;
    bool showMotionBlur = true;
};


#endif //PBL_RENDERINGSYSTEM_H
