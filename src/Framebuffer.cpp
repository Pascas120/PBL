#include "Framebuffer.h"

#include <spdlog/spdlog.h>
#include <GLFW/glfw3.h>

Framebuffer::~Framebuffer() = default;

void DefaultFramebuffer::Bind() const
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DefaultFramebuffer::GetSize(uint32_t& width, uint32_t& height) const
{
	glfwGetFramebufferSize(glfwGetCurrentContext(), (int*)&width, (int*)&height);
}

CustomFramebuffer::CustomFramebuffer(const FramebufferConfig& config) : config(config)
{
	Setup();
}

CustomFramebuffer::~CustomFramebuffer()
{
	Clear();
}

void CustomFramebuffer::Clear()
{
	glDeleteFramebuffers(1, &fbo);
	for (const auto& [attachment, texture] : textures)
	{
		glDeleteTextures(1, &texture);
	}
	textures.clear();
}

void CustomFramebuffer::Setup()
{
	if (fbo)
	{
		Clear();
	}

	GLint currentFramebuffer;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFramebuffer);


	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	int colorAttachment = GL_COLOR_ATTACHMENT0;


	// Color
	if (std::find(config.attachments.begin(), config.attachments.end(), AttachmentType::COLOR) != config.attachments.end())
	{
		GLuint colorTexture;
		glGenTextures(1, &colorTexture);
		glBindTexture(GL_TEXTURE_2D, colorTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, config.colorFormat, config.width, config.height, 0, config.colorFormat, config.colorType, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glFramebufferTexture2D(GL_FRAMEBUFFER, colorAttachment, GL_TEXTURE_2D, colorTexture, 0);
		colorAttachment++;

		textures[AttachmentType::COLOR] = colorTexture;
	}

	// Position
	if (std::find(config.attachments.begin(), config.attachments.end(), AttachmentType::POSITION) != config.attachments.end())
	{
		GLuint positionTexture;
		glGenTextures(1, &positionTexture);
		glBindTexture(GL_TEXTURE_2D, positionTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, config.width, config.height, 0, GL_RGB, GL_HALF_FLOAT, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glFramebufferTexture2D(GL_FRAMEBUFFER, colorAttachment, GL_TEXTURE_2D, positionTexture, 0);
		colorAttachment++;

		textures[AttachmentType::POSITION] = positionTexture;
	}

	// Normal
	if (std::find(config.attachments.begin(), config.attachments.end(), AttachmentType::NORMAL) != config.attachments.end())
	{
		GLuint normalTexture;
		glGenTextures(1, &normalTexture);
		glBindTexture(GL_TEXTURE_2D, normalTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, config.width, config.height, 0, GL_RGB, GL_HALF_FLOAT, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glFramebufferTexture2D(GL_FRAMEBUFFER, colorAttachment, GL_TEXTURE_2D, normalTexture, 0);
		colorAttachment++;

		textures[AttachmentType::NORMAL] = normalTexture;
	}


	// Velocity
	if (std::find(config.attachments.begin(), config.attachments.end(), AttachmentType::VELOCITY) != config.attachments.end())
	{
		GLuint velocityTexture;
		glGenTextures(1, &velocityTexture);
		glBindTexture(GL_TEXTURE_2D, velocityTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, config.width, config.height, 0, GL_RG, GL_HALF_FLOAT, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glFramebufferTexture2D(GL_FRAMEBUFFER, colorAttachment, GL_TEXTURE_2D, velocityTexture, 0);
		colorAttachment++;

		textures[AttachmentType::VELOCITY] = velocityTexture;
	}


	// Depth
	if (std::find(config.attachments.begin(), config.attachments.end(), AttachmentType::DEPTH) != config.attachments.end())
	{
		GLuint depthTexture;
		glGenTextures(1, &depthTexture);
		glBindTexture(GL_TEXTURE_2D, depthTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, config.width, config.height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);

		textures[AttachmentType::DEPTH] = depthTexture;
	}

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		spdlog::error("Framebuffer is not complete!");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, currentFramebuffer);
}

void CustomFramebuffer::Bind() const
{
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glViewport(0, 0, config.width, config.height);
}

void CustomFramebuffer::Resize(uint32_t width, uint32_t height)
{
	config.width = width;
	config.height = height;

	Setup();
}