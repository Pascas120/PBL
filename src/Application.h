#pragma once

#define IMGUI_IMPL_OPENGL_LOADER_GLAD

#include <GLFW/glfw3.h> // Include glfw3.h after our OpenGL definitions
#include <glm/glm.hpp>
#include <memory>

#include "Framebuffer.h"
#include "Camera.h"
#include "Shader.h"
#include "Model.h"
#include "Scene.h"

class Application
{
public:
	Application();
	virtual ~Application();
	virtual void run();

	void input();
	void update();
	void render(const Framebuffer& framebuffer = DefaultFramebuffer::GetInstance());
	void render(Camera& camera, const Framebuffer& framebuffer = DefaultFramebuffer::GetInstance());

	std::pair<double, double> getScrollOffset() const
	{
		return { scrollXOffset, scrollYOffset };
	}

	float getDeltaTime() const
	{
		return deltaTime;
	}

protected:
	bool init();
	
	// TODO: different name?
	void renderToWindow();
	void endFrame();

	const char* glsl_version = "#version 410";
	const int32_t GL_VERSION_MAJOR = 4;
	const int32_t GL_VERSION_MINOR = 1;

	static constexpr int32_t WINDOW_WIDTH = 1920;
	static constexpr int32_t WINDOW_HEIGHT = 1080;

	GLFWwindow* window = nullptr;

	double scrollXOffset = 0.0f;
	double scrollYOffset = 0.0f;

	float deltaTime = 0.0f;
	float lastFrame = 0.0f;

	// TODO: move to camera component
	Camera camera{ glm::vec3(0.0f, 0.0f, 3.0f) };

	// TODO: move to resource manager(s?)
	std::vector<Shader*> shaders;
	std::vector<Model*> models;

	std::shared_ptr<Scene> scene;

	// TODO: player component
	EntityID player = (EntityID)-1;

private:
	// TODO: remove
	void setupScene();
};