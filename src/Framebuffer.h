#pragma once

#include <cassert>
#include <glad/glad.h>
#include <stdint.h>
#include <utility>
#include <set>
#include <map>

class Framebuffer
{
public:
	virtual ~Framebuffer() = 0;

	virtual void Bind() const = 0;
	virtual void GetSize(uint32_t& width, uint32_t& height) const = 0;
	virtual GLuint GetColorTexture() const = 0;
	virtual GLuint GetDepthTexture() const = 0;
	std::pair<uint32_t, uint32_t> GetSizePair() const
	{
		uint32_t width, height;
		GetSize(width, height);
		return { width, height };
	}
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
	GLuint GetColorTexture() const override {assert(false && "Default framebuffer does not have a color texture."); return 0; };
	GLuint GetDepthTexture() const override {assert(false && "Default framebuffer does not have a color texture."); return 0; };

	static DefaultFramebuffer& GetInstance()
	{
		static DefaultFramebuffer instance;
		return instance;
	}
};


enum class AttachmentType
{
	COLOR,
	DEPTH,
	POSITION,
	NORMAL,
	VELOCITY
};

struct FramebufferConfig
{
	uint32_t width;
	uint32_t height;
	std::set<AttachmentType> attachments { AttachmentType::COLOR, AttachmentType::DEPTH };
	GLenum colorFormat = GL_RGB;
	GLenum colorType = GL_UNSIGNED_BYTE;
};

class CustomFramebuffer : public Framebuffer
{
private:
	GLuint fbo = 0;
	/*GLuint colorTexture = 0;
	GLuint depthTexture = 0;
	GLuint velocityTexture = 0;*/
	std::map<AttachmentType, GLuint> textures;


	FramebufferConfig config;

	void Setup();
	void Clear();

public:
	CustomFramebuffer(const FramebufferConfig& config);
	~CustomFramebuffer();

	void Bind() const;

	void Resize(uint32_t width, uint32_t height);
	GLuint GetColorTexture() const override { return GetTexture(AttachmentType::COLOR); }
	GLuint GetDepthTexture() const override { return GetTexture(AttachmentType::DEPTH); }
	GLuint GetVelocityTexture() const { return GetTexture(AttachmentType::VELOCITY); }
	GLuint GetNormalTexture() const { return GetTexture(AttachmentType::NORMAL); }
	GLuint GetPositionTexture() const { return GetTexture(AttachmentType::POSITION); }

	GLuint GetTexture(AttachmentType attachment) const
	{
		auto it = textures.find(attachment);
		if (it != textures.end())
		{
			return it->second;
		}
		return 0;
	}

	const FramebufferConfig& GetConfig() const { return config; }
	void GetSize(uint32_t& width, uint32_t& height) const
	{
		width = config.width;
		height = config.height;
	}
};