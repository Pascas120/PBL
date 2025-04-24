#pragma once

#include <glad/glad.h>
#include <stdint.h>
#include <utility>

class Framebuffer
{
public:
	virtual ~Framebuffer() = 0;

	virtual void Bind() const = 0;
	virtual void GetSize(uint32_t& width, uint32_t& height) const = 0;
};

class DefaultFramebuffer : public Framebuffer
{
private:
	DefaultFramebuffer() = default;

public:
	DefaultFramebuffer(const DefaultFramebuffer&) = delete;
	DefaultFramebuffer& operator=(const DefaultFramebuffer&) = delete;
	DefaultFramebuffer(DefaultFramebuffer&&) = delete;
	~DefaultFramebuffer() override = default;

	void Bind() const;
	void GetSize(uint32_t& width, uint32_t& height) const;

	static DefaultFramebuffer& GetInstance()
	{
		static DefaultFramebuffer instance;
		return instance;
	}
};



struct FramebufferConfig
{
	uint32_t width;
	uint32_t height;
};

class CustomFramebuffer : public Framebuffer
{
private:
	GLuint fbo = 0;
	GLuint colorTexture = 0;
	GLuint depthTexture = 0;

	FramebufferConfig config;

	void Setup();
	void Clear() const;

public:
	CustomFramebuffer(const FramebufferConfig& config);
	~CustomFramebuffer();

	void Bind() const;

	void Resize(uint32_t width, uint32_t height);
	GLuint GetColorTexture() const { return colorTexture; }
	GLuint GetDepthTexture() const { return depthTexture; }

	const FramebufferConfig& GetConfig() const { return config; }
	void GetSize(uint32_t& width, uint32_t& height) const
	{
		width = config.width;
		height = config.height;
	}
};