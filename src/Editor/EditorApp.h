#pragma once

#include "Application.h"
#include "nlohmann/json.hpp"
#include "ECS/EventSystem.h"

using json = nlohmann::json;

namespace Editor
{
	class HierarchyWindow;
	class InspectorWindow;
	class ShaderWindow;
	class SceneWindow;
	class GameWindow;

	enum class PlayMode
	{
		PLAY,
		PAUSE,
		STOP
	};

	struct EditorClipboard
	{
		json objectJson;
	};

	class EditorApp : public Application, public std::enable_shared_from_this<EditorApp>
	{
	public:
		EditorApp();
		~EditorApp();
		void run() override;

		EntityID selectedObject = (EntityID)-1;

		PlayMode playMode = PlayMode::STOP;
		void setPlayMode(PlayMode mode);

		EditorClipboard clipboard;

		EventSystem& getEventSystem() { return editorEventSystem; }

	protected:
		void initImGui();

		void createImGuiDrawData();
		void renderImGui();

		std::string scenePath = "";
		void saveScene(std::string path = "");
		void loadScene();

		std::shared_ptr<Scene> sceneBackup = nullptr;

		EventSystem editorEventSystem;

		std::unique_ptr<HierarchyWindow> hierarchyWindow;
		std::unique_ptr<InspectorWindow> inspectorWindow;
		std::unique_ptr<ShaderWindow> shaderWindow;

		std::unique_ptr<SceneWindow> sceneWindow;
		std::unique_ptr<GameWindow> gameWindow;

	};


	struct EditorContext
	{
		std::shared_ptr<Scene> scene;
		std::vector<Shader*>& shaders;
		std::vector<Model*>& models;
	};

	namespace Payload
	{
		static const char* HIERARCHY_NODE = "HIERARCHY_NODE";
	}
}