//
// Created by Łukasz Moskwin on 29/03/2025.
//

#ifndef PBL_MODEL_H
#define PBL_MODEL_H

#include <vector>
#include <string>
#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "stb_image.h"
#include "Mesh.h"
#include "Shader.h"

class Model
{
public:
    std::vector<Mesh> meshes;
    std::string directory;
    bool gammaCorrection;

    Model();
    Model(std::string const &path, bool gamma = false);
    void Draw(Shader *shader);

private:
    void loadModel(std::string const &path);
    void processNode(aiNode *node, const aiScene *scene);
    Mesh processMesh(aiMesh *mesh, const aiScene *scene);
    std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName);
};

unsigned int TextureFromFile(const char *path, const std::string &directory, bool gamma = false);


#endif //PBL_MODEL_H
