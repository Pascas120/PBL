#pragma once

#include <glad/glad.h>
#include <stdint.h>


struct FramebufferConfig
{
	uint32_t width;
	uint32_t height;
};

class Framebuffer
{
private:
	GLuint fbo = 0;
	GLuint colorTexture = 0;
	GLuint depthTexture = 0;

	FramebufferConfig config;

	void Setup();
	void Clear() const;

public:
	Framebuffer(const FramebufferConfig& config);
	~Framebuffer();

	void Bind() const;
	void Unbind() const;

	void Resize(uint32_t width, uint32_t height);
	GLuint GetColorTexture() const { return colorTexture; }
	GLuint GetDepthTexture() const { return depthTexture; }

	const FramebufferConfig& GetConfig() const { return config; }
};