#pragma once

#include "Application.h"

namespace Editor
{
	class HierarchyWindow;
	class InspectorWindow;
	class ShaderWindow;

	class EditorApp : public Application, public std::enable_shared_from_this<EditorApp>
	{
	public:
		EditorApp();
		~EditorApp();
		void run() override;

		EntityID selectedObject = (EntityID)-1;

	protected:
		void initImGui();

		void createImGuiDrawData();
		void renderImGui();

		std::unique_ptr<CustomFramebuffer> sceneFramebuffer;

		// TODO: scene window class?
		void imguiScene();

		std::unique_ptr<HierarchyWindow> hierarchyWindow = std::make_unique<HierarchyWindow>();
		std::unique_ptr<InspectorWindow> inspectorWindow = std::make_unique<InspectorWindow>();
		std::unique_ptr<ShaderWindow> shaderWindow = std::make_unique<ShaderWindow>();
	};


	struct EditorContext
	{
		EditorApp* editor;
		std::shared_ptr<Scene> scene;
		std::vector<Shader*>& shaders;
		std::vector<Model*>& models;
	};

	namespace Payload
	{
		static const char* HIERARCHY_NODE = "HIERARCHY_NODE";
	}
}