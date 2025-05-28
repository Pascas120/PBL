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
#include "Bone.h"
#include <map>

#include "ECS/BoundingVolumes.h"

class BoundingBox;

class Model
{
public:
    std::vector<Mesh> meshes;
    std::string directory;
    std::string path;
    bool gammaCorrection;

    Model();
    Model(std::string const &path, bool gamma = false);
    void draw(Shader *shader);
    std::map<std::string, BoneInfo>& getBoneInfoMap();
    int& getBoneCount();

    BoundingBox boundingBox;

private:
    void loadModel(std::string const &path);
    void processNode(aiNode *node, const aiScene *scene);
    Mesh processMesh(aiMesh *mesh, const aiScene *scene, glm::mat4 transform);
    std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName);
    std::map<std::string, BoneInfo> boneInfoMap; //
    int boneCount = 0;

    void setVertexBoneDataToDefault(Vertex& vertex);
    void setVertexBoneData(Vertex& vertex, int boneID, float weight);
    void extractBoneWeightForVertices(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene);
};

unsigned int TextureFromFile(const char *path, const std::string &directory, bool gamma = false);



#endif //PBL_MODEL_H
