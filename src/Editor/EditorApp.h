#pragma once

#include "Application.h"

//// include w headerze nie kompiluje siê (idk czemu)
//namespace ImGuizmo {
//	enum OPERATION : int;
//}

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

	class EditorApp : public Application, public std::enable_shared_from_this<EditorApp>
	{
	public:
		EditorApp();
		~EditorApp();
		void run() override;

		EntityID selectedObject = (EntityID)-1;

		PlayMode playMode = PlayMode::STOP;
		void setPlayMode(PlayMode mode);

	protected:
		void initImGui();

		void createImGuiDrawData();
		void renderImGui();

		std::string scenePath = "";
		void saveScene(std::string path = "");
		void loadScene();

		std::shared_ptr<Scene> sceneBackup = nullptr;

		std::unique_ptr<HierarchyWindow> hierarchyWindow = std::make_unique<HierarchyWindow>();
		std::unique_ptr<InspectorWindow> inspectorWindow = std::make_unique<InspectorWindow>();
		std::unique_ptr<ShaderWindow> shaderWindow = std::make_unique<ShaderWindow>();

		std::unique_ptr<SceneWindow> sceneWindow = std::make_unique<SceneWindow>();
		std::unique_ptr<GameWindow> gameWindow = std::make_unique<GameWindow>();
	};


	struct EditorContext
	{
		EditorApp* editor;
		std::shared_ptr<Scene> scene;
		std::vector<Shader*>& shaders;
		std::vector<Model*>& models;
		Camera& camera;
	};

	namespace Payload
	{
		static const char* HIERARCHY_NODE = "HIERARCHY_NODE";
	}
}