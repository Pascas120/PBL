#include "UniformBuffer.h"

#include <vector>
#include <spdlog/spdlog.h>

#define GL_CHECK(x) \
    x; \
    { GLenum err = glGetError(); if (err != GL_NO_ERROR) spdlog::error("OpenGL error after "#x": {}", err); }


UniformBlock::UniformBlock(const std::string& blockName, GLuint bindingPoint)
	: blockName(blockName), bindingPoint(bindingPoint) {
}

UniformBlock::~UniformBlock() {
	if (isInitialized())
		glDeleteBuffers(1, &uboID);
}

void UniformBlock::setData(const std::string& name, const void* data) const {
	if (!isInitialized()) {
		spdlog::error("Uniform block '{}' is not initialized.", blockName);
		return;
	}

	auto it = uniformOffsets.find(name);
	if (it == uniformOffsets.end()) {
		spdlog::warn("Uniform '{}' not found in block '{}'.", name, blockName);
		return;
	}

	const auto& [offset, size, count, stride] = it->second;

	glBindBuffer(GL_UNIFORM_BUFFER, uboID);
	glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UniformBlock::setArrayData(const std::string& name, const void* data, GLint index) const {
	if (!isInitialized()) {
		spdlog::error("Uniform block '{}' is not initialized.", blockName);
		return;
	}
	auto it = uniformOffsets.find(name + "[0]");
	if (it == uniformOffsets.end()) {
		spdlog::warn("Uniform '{}' not found in block '{}'.", name, blockName);
		return;
	}
	const auto& [offset, size, arraySize, stride] = it->second;
	if (index < 0) {
		index = 0;
	}
	else if (index >= arraySize) {
		index = arraySize - 1;
	}
	glBindBuffer(GL_UNIFORM_BUFFER, uboID);
	GLintptr arrayOffset = offset + index * stride;
	glBufferSubData(GL_UNIFORM_BUFFER, arrayOffset, size, data);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UniformBlock::bindToShader(GLuint shaderProgram) const {
	if (!isInitialized()) {
		spdlog::error("Uniform block '{}' is not initialized.", blockName);
		return;
	}
	GLuint blockIndex = glGetUniformBlockIndex(shaderProgram, blockName.c_str());
	if (blockIndex == GL_INVALID_INDEX) {
		spdlog::error("Uniform block '{}' not found in shader program.", blockName);
		return;
	}
	glUniformBlockBinding(shaderProgram, blockIndex, bindingPoint);
	glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, uboID);
}

static GLsizeiptr getGLTypeSize(GLenum type);

void UniformBlock::init(GLuint shaderProgram) {
	if (isInitialized()) {
		spdlog::warn("Uniform block '{}' is already initialized.", blockName);
		return;
	}

	GLuint blockIndex = glGetUniformBlockIndex(shaderProgram, blockName.c_str());
	if (blockIndex == GL_INVALID_INDEX) {
		spdlog::error("Uniform block '{}' not found in shader program.", blockName);
		return;
	}

	GLint blockSizeInt = 0;
	glGetActiveUniformBlockiv(shaderProgram, blockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSizeInt);
	blockSize = (GLsizeiptr)blockSizeInt;

	glGenBuffers(1, &uboID);
	glBindBuffer(GL_UNIFORM_BUFFER, uboID);
	GL_CHECK(
		glBufferData(GL_UNIFORM_BUFFER, blockSize, nullptr, GL_DYNAMIC_DRAW);
	);
	glBindBufferRange(GL_UNIFORM_BUFFER, bindingPoint, uboID, 0, blockSize);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glUniformBlockBinding(shaderProgram, blockIndex, bindingPoint);

	GLint numUniforms;
	glGetActiveUniformBlockiv(shaderProgram, blockIndex, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &numUniforms);

	std::vector<GLint> indices(numUniforms);
	glGetActiveUniformBlockiv(shaderProgram, blockIndex, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, indices.data());

	std::vector<GLint> offsets(numUniforms);
	std::vector<GLint> types(numUniforms);
	std::vector<GLint> arraySizes(numUniforms);
	std::vector<GLint> strides(numUniforms);

	glGetActiveUniformsiv(shaderProgram, numUniforms, reinterpret_cast<const GLuint*>(indices.data()), GL_UNIFORM_OFFSET, offsets.data());
	glGetActiveUniformsiv(shaderProgram, numUniforms, reinterpret_cast<const GLuint*>(indices.data()), GL_UNIFORM_TYPE, types.data());
	glGetActiveUniformsiv(shaderProgram, numUniforms, reinterpret_cast<const GLuint*>(indices.data()), GL_UNIFORM_SIZE, arraySizes.data());
	glGetActiveUniformsiv(shaderProgram, numUniforms, reinterpret_cast<const GLuint*>(indices.data()), GL_UNIFORM_ARRAY_STRIDE, strides.data());

	for (int i = 0; i < numUniforms; ++i) {
		char name[256];
		GLsizei length;
		glGetActiveUniformName(shaderProgram, indices[i], sizeof(name), &length, name);

		GLsizeiptr sizeInBytes = getGLTypeSize(types[i]) * arraySizes[i];

		uniformOffsets[name] = { offsets[i], sizeInBytes, (GLuint)arraySizes[i], (GLuint)strides[i] };

	}
}



static GLsizeiptr getGLTypeSize(GLenum type) {
	switch (type) {
	case GL_FLOAT: return sizeof(GLfloat);
	case GL_FLOAT_VEC2: return sizeof(GLfloat) * 2;
	case GL_FLOAT_VEC3: return sizeof(GLfloat) * 4;
	case GL_FLOAT_VEC4: return sizeof(GLfloat) * 4;
	case GL_INT: return sizeof(GLint);
	case GL_INT_VEC2: return sizeof(GLint) * 2;
	case GL_INT_VEC3: return sizeof(GLint) * 4;
	case GL_INT_VEC4: return sizeof(GLint) * 4;
	case GL_FLOAT_MAT4: return sizeof(GLfloat) * 16;
	case GL_FLOAT_MAT3: return sizeof(GLfloat) * 12;
	default:
		spdlog::warn("Unknown GL uniform type: {}", type);
		return 0;
	}
}