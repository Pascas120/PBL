#pragma once

#include "Application.h"
#include "ECS/EventSystem.h"
#include "Utils/Debug.h"


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

		Utils::Debug getDebug() { return debug; }

		Shader* getEditorShader(const std::string& name) const;

		void prefabWindow();

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


		std::unordered_map<std::string, Shader*> editorShaders;

		
		Utils::Debug debug;
	};


	struct EditorContext
	{
		std::shared_ptr<Scene> scene;
		std::vector<Shader*>& shaders;
		std::vector<Model*>& models;
		std::vector<std::string>& sounds;
	};

	namespace Payload
	{
		static const char* HIERARCHY_NODE = "HIERARCHY_NODE";
	}
}