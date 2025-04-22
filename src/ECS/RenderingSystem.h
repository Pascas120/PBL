//
// Created by Łukasz Moskwin on 15/04/2025.
//

#ifndef PBL_RENDERINGSYSTEM_H
#define PBL_RENDERINGSYSTEM_H


#include <map>

#include "Scene.h"
#include "Camera.h"
#include "Shader.h"
#include "TextRenderer.h"

class RenderingSystem{
private:
    Scene* scene;
    Shader sceneShader;
    Shader hudShader;
    Shader textShader;

    std::map<std::string, GLuint> textures;
    GLuint getTexture(std::string path);
    GLuint hudVAO, hudVBO, hudEBO;
    bool initializedHud = false;
    void initHud();
    TextRenderer t1;

public:
    RenderingSystem();
    RenderingSystem(Scene* scene, Shader &sceneShader, Shader &hudShader, Shader &textShader);
    void drawScene(Camera& camera);
    void drawHud();
    void draw(Camera& camera);
};


#endif //PBL_RENDERINGSYSTEM_H
