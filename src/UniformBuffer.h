#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>

struct UniformOffset {
    GLintptr offset;
    GLsizeiptr size;
    GLuint count;
    GLuint stride;
};

class UniformBlock {
public:
    UniformBlock(const std::string& blockName, GLuint bindingPoint);
    ~UniformBlock();

    void setData(const std::string& name, const void* data) const;
    void setArrayData(const std::string& name, const void* data, GLint index) const;
    void bindToShader(GLuint shaderProgram) const;

    void init(GLuint shaderProgram);

    bool isInitialized() const {
        return uboID != 0;
    }

private:
    std::string blockName;
    GLuint uboID = 0;
    GLuint bindingPoint;
    GLsizeiptr blockSize;
    std::unordered_map<std::string, UniformOffset> uniformOffsets;
};


struct UniformBlockStorage {
    UniformBlock cameraBlock{ "Camera", 0 };
    UniformBlock lightBlock{ "Lights", 1 };
};