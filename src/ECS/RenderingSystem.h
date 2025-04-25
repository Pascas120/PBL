//
// Created by Łukasz Moskwin on 15/04/2025.
//

#ifndef PBL_RENDERINGSYSTEM_H
#define PBL_RENDERINGSYSTEM_H


#include <map>

class Scene;
#include "Camera.h"
#include "Shader.h"
#include "TextRenderer.h"
#include "Framebuffer.h"

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

public:
    //RenderingSystem(Scene* scene, Shader &sceneShader, Shader &hudShader, Shader &textShader);
    RenderingSystem(Scene* scene);
    void drawScene(const Framebuffer& framebuffer, const Camera& camera);
    void drawHud(const Framebuffer& framebuffer);
};


#endif //PBL_RENDERINGSYSTEM_H
