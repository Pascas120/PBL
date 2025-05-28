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

void CustomFramebuffer::Clear() const
{
	glDeleteFramebuffers(1, &fbo);
	glDeleteTextures(1, &colorTexture);
	glDeleteTextures(1, &depthTexture);
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


	// Color
	if (std::find(config.attachments.begin(), config.attachments.end(), AttachmentType::COLOR) != config.attachments.end())
	{
		glGenTextures(1, &colorTexture);
		glBindTexture(GL_TEXTURE_2D, colorTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, config.width, config.height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);
	}


	// Velocity
	if (std::find(config.attachments.begin(), config.attachments.end(), AttachmentType::VELOCITY) != config.attachments.end())
	{
		spdlog::info("Creating velocity texture with size {}x{}", config.width, config.height);
		glGenTextures(1, &velocityTexture);
		glBindTexture(GL_TEXTURE_2D, velocityTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, config.width, config.height, 0, GL_RG, GL_HALF_FLOAT, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, velocityTexture, 0);
	}


	// Depth
	if (std::find(config.attachments.begin(), config.attachments.end(), AttachmentType::DEPTH) != config.attachments.end())
	{
		glGenTextures(1, &depthTexture);
		glBindTexture(GL_TEXTURE_2D, depthTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, config.width, config.height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
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