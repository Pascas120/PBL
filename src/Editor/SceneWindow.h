#pragma once 
#include "EditorApp.h"
#include "imgui.h"
#include "ImGuizmo.h"

 
namespace Editor
{
	class SceneWindow
	{
	public:
		SceneWindow();

		void draw(const EditorContext& context);

		float camDistance = 10.0f;
		float cameraSpeed = 1.0f;

	private:
		void drawWindow(const EditorContext& context);
		void cameraControls(double scrollYOffset, float deltaTime);

		std::unique_ptr<CustomFramebuffer> sceneFramebuffer;
		ImVec2 lastSize = ImVec2(0, 0);

		ImGuizmo::OPERATION gizmoOperation;
		Camera editorCamera{ glm::vec3(0.0f, 0.0f, 3.0f) };
	};
}