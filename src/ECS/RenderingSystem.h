//
// Created by Łukasz Moskwin on 15/04/2025.
//

#ifndef PBL_RENDERINGSYSTEM_H
#define PBL_RENDERINGSYSTEM_H


#include <map>

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
    std::unordered_map<std::string, const Shader*> postShaders;

    void sobelFilter(const CustomFramebuffer &in, const Framebuffer &out);
public:
    //RenderingSystem(Scene* scene, Shader &sceneShader, Shader &hudShader, Shader &textShader);
    RenderingSystem(Scene* scene);
    void drawScene(const Framebuffer& framebuffer, Camera& camera, const UniformBlockStorage& uniformBlockStorage);
    void drawHud(const Framebuffer& framebuffer);
    void buildTree();
    void addPostShader(const std::string& name, const Shader* shader);

	bool useTree = true;
};


#endif //PBL_RENDERINGSYSTEM_H
