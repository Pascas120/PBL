#pragma once

#define IMGUI_IMPL_OPENGL_LOADER_GLAD

#include <GLFW/glfw3.h> // Include glfw3.h after our OpenGL definitions
#include <glm/glm.hpp>
#include <memory>
#include <unordered_map>
#include <queue>

#include "Framebuffer.h"
#include "Camera.h"
#include "Shader.h"
#include "Model.h"
#include "Scene.h"

#include "UniformBuffer.h"

#include "ECS/TransformSystem.h"

#include "nlohmann/json.hpp"

using json = nlohmann::json;

struct DialogueCharacter
{
	std::string name;
	std::string imagePath;
};

struct DialogueBox
{
	std::string character;
	std::array<std::string, 3> lines;
};

struct DialogueBoxEntities
{
	EntityID background = (EntityID)-1;
	EntityID icon = (EntityID)-1;
	EntityID name = (EntityID)-1;
	EntityID nameShadow = (EntityID)-1;
	std::array<EntityID, 3> lines = { (EntityID)-1, (EntityID)-1, (EntityID)-1 };
};

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

	std::vector<EntityID> instantiatePrefab(const std::string& prefabName, Scene& scene, EntityID parent = (EntityID)-1);

protected:
	bool init();
	
	// TODO: different name?
	void renderToWindow();
	void endFrame();

	const std::string firstScene = "res/scenes/poziom1.json";
	std::string scenePath = "";
	std::string changeSceneTo = "";
	virtual void loadScene(const std::string path);

	int currentLevel = 0;

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



	// TODO: move to resource manager(s?)
	std::vector<Shader*> shaders;
	std::unordered_map<std::string, Shader*> postShaders;

	UniformBlockStorage uniformBlockStorage;

	std::unordered_map<std::string, UniformBlock*> uniformBlockMap{
		{ "Camera", &uniformBlockStorage.cameraBlock },
		{ "Lights", &uniformBlockStorage.lightBlock },
		{ "SplitScreen", &uniformBlockStorage.splitScreenBlock }
	};

	std::vector<std::string> sounds;

	std::vector<Model*> models;

	std::vector<Animation*> animations;

	std::unordered_map<std::string, json> prefabs;

	std::shared_ptr<Scene> scene;

	// TODO: player component
	EntityID player = (EntityID)-1;

	void setupEvents();
	void setStartValues();

	std::vector<std::string> jokes;

	std::unordered_map<std::string, DialogueCharacter> dialogueCharacterInfo;
	std::unordered_map<std::string, std::vector<DialogueBox>> dialogues;
	std::queue<DialogueBox> currentDialogue;
	DialogueBoxEntities dialogueBoxEntities;
	bool updateDialogueBox = true;
	bool dialogueAdvanceKeyPressed = false;
	EntityID ingredient = (EntityID)-1;
	float ingredientAnimationTimer;

private:
	// TODO: remove
	void setupScene();

public:
	ma_engine audioEngine;
	std::map<std::string, GLuint> textures;

};