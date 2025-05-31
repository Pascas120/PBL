#pragma once 
#include "EditorApp.h"
#include "Utils/editor_events.h"

#include "imgui.h"
#include "ImGuizmo.h"


 
namespace Editor
{
	class SceneWindow
	{
	public:
		SceneWindow(EditorApp* editor);
		~SceneWindow();

		void draw(const EditorContext& context);

		float camDistance = 10.0f;
		float cameraSpeed = 1.0f;

	private:
		EditorApp* editor = nullptr;

		void drawWindow(const EditorContext& context);
		void cameraControls(double scrollYOffset, float deltaTime);

		EventListener focusCamera = [this](const Event& event)
			{
				const Events::CameraFocus& cameraFocusEvent = static_cast<const Events::CameraFocus&>(event);

				auto camVecAng = editorCamera.getVectorsAndAngles();
				glm::vec3 newPos = cameraFocusEvent.position - camVecAng.forward * camDistance;
				editorCamera.setViewMatrix(glm::lookAt(newPos, cameraFocusEvent.position, camVecAng.up));
			};

		std::unique_ptr<CustomFramebuffer> sceneFramebuffer;
		ImVec2 lastSize = ImVec2(0, 0);

		ImGuizmo::OPERATION gizmoOperation = ImGuizmo::OPERATION::UNIVERSAL;
		ImGuizmo::MODE gizmoMode = ImGuizmo::MODE::LOCAL;
		Camera editorCamera;
	};
}