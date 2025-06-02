#include "SceneWindow.h" 
#include "Utils/editor_utils.h"

#include <spdlog/spdlog.h>

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl/imgui_impl_glfw.h"
#include "imgui_impl/imgui_impl_opengl3.h"

#include "ImGuizmo.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
 
namespace Editor
{
	SceneWindow::SceneWindow(EditorApp* editor) : editor(editor)
	{
		sceneFramebuffer = std::make_unique<CustomFramebuffer>(FramebufferConfig{ 0, 0 });
        editorCamera.setViewMatrix(glm::lookAt(
            glm::vec3(0.0f, 0.0f, camDistance),
            glm::vec3(0.0f, 0.0f, 1.0f),
            glm::vec3(0.0f, 1.0f, 0.0f)));

		editorCamera.getFrustum().setProjectionMatrix(
			glm::perspective(glm::radians(45.0f), 650.0f / 400.0f, 0.1f, 100.0f));

		editor->getEventSystem().registerListener<Events::CameraFocus>(focusCamera);
	}

    SceneWindow::~SceneWindow()
    {
		editor->getEventSystem().unregisterListener<Events::CameraFocus>(focusCamera);
    }

    void SceneWindow::draw(const EditorContext& context)
    {
		drawWindow(context);
    }

	static std::string getGizmoOperationName(ImGuizmo::OPERATION operation)
	{
		switch (operation)
		{
		case ImGuizmo::TRANSLATE: return "Translate";
		case ImGuizmo::ROTATE: return "Rotate";
		case ImGuizmo::SCALE: return "Scale";
		case ImGuizmo::UNIVERSAL: return "Universal";
		default: return "Unknown";
		}
	}

	static std::string getGizmoModeName(ImGuizmo::MODE mode)
	{
		switch (mode)
		{
		case ImGuizmo::LOCAL: return "Local";
		case ImGuizmo::WORLD: return "World";
		default: return "Unknown";
		}
	}

    void SceneWindow::drawWindow(const EditorContext& context)
    {
		auto& scene = context.scene;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(650, 400), ImGuiCond_FirstUseEver);

        if (ImGui::Begin("Scene", NULL, ImGuiWindowFlags_MenuBar))
        {
           if (ImGui::BeginMenuBar())
           {
			   ImGui::Text("Gizmo");

			   static ImGuizmo::OPERATION gizmoOperations[4] = {
				   ImGuizmo::TRANSLATE,
				   ImGuizmo::ROTATE,
				   ImGuizmo::SCALE,
				   ImGuizmo::UNIVERSAL
			   };
			   ImGui::SetNextItemWidth(100);
			   if (ImGui::BeginCombo("##gizmoOperation", getGizmoOperationName(gizmoOperation).c_str()))
			   {
                   for (auto op : gizmoOperations)
                   {
                       bool isSelected = (gizmoOperation == op);
                       if (ImGui::Selectable(getGizmoOperationName(op).c_str(), isSelected))
                       {
                           gizmoOperation = op;
                       }
                   }
				   ImGui::EndCombo();
			   }

			   static ImGuizmo::MODE gizmoModes[2] = {
				   ImGuizmo::LOCAL,
				   ImGuizmo::WORLD
			   };

               ImGui::SetNextItemWidth(100);
               if (ImGui::BeginCombo("##gizmoMode", getGizmoModeName(gizmoMode).c_str()))
			   {
				   for (auto mode : gizmoModes)
				   {
					   bool isSelected = (gizmoMode == mode);
					   if (ImGui::Selectable(getGizmoModeName(mode).c_str(), isSelected))
					   {
						   gizmoMode = mode;
					   }
				   }
				   ImGui::EndCombo();
			   }

               ImGui::Checkbox("Colliders", &drawColliders);

               ImGui::EndMenuBar();
           }

            ImVec2 pos = ImGui::GetCursorScreenPos();
            ImVec2 size = ImGui::GetContentRegionAvail();


            if (ImGui::IsWindowHovered() &&
                (ImGui::IsMouseClicked(ImGuiMouseButton_Right) || ImGui::IsMouseClicked(ImGuiMouseButton_Middle)))
            {
                ImGui::SetWindowFocus();
            }

            // TODO: fix (doesn't work with imguizmo)
            /*if (ImGui::IsWindowFocused() && (ImGuizmo::IsUsingAny() || ImGuizmo::IsUsingViewManipulate()))
            {
                Utils::wrapCursor(pos, size);
            }*/

            if (size.x != lastSize.x || size.y != lastSize.y)
            {
                sceneFramebuffer->Resize(size.x, size.y);

				editorCamera.getFrustum().setProjectionMatrix(
					glm::perspective(glm::radians(45.0f), size.x / size.y, 0.1f, 100.0f));
                lastSize = size;
            }

            scene->getRenderingSystem().useTree = false;
			scene->getRenderingSystem().showMotionBlur = false;
            editor->render(editorCamera, *sceneFramebuffer);

            if (drawColliders && scene->hasEntity(editor->selectedObject)
                && scene->hasComponent<ColliderComponent>(editor->selectedObject))
            {
                editor->getDebug().drawCollider(
                    scene->getComponent<ColliderComponent>(editor->selectedObject).GetColliderShape(),
                    scene->getComponent<Transform>(editor->selectedObject),
                    editor->getEditorShader("Flat")
                );
            }

            GLuint texture = sceneFramebuffer->GetColorTexture();
            ImGui::Image(texture, size, ImVec2(0, 1), ImVec2(1, 0));


            bool disableGizmo = false;
			auto camVecAng = editorCamera.getVectorsAndAngles();

            glm::vec3 cameraFocus = camVecAng.position + camVecAng.forward * camDistance;
            double scrollYOffset = editor->getScrollOffset().second;

            if (ImGui::IsWindowFocused() && ImGui::IsWindowHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Right))
            {
				cameraControls(scrollYOffset, editor->getDeltaTime());
                //Utils::wrapCursor(pos, size);
                disableGizmo = true;
                ImGui::ResetMouseDragDelta(ImGuiMouseButton_Right);
            }
            else if (ImGui::IsWindowHovered() && !Utils::isActiveInAnotherWindow())
            {
                if (scrollYOffset != 0.0f)
                {
                    camDistance -= scrollYOffset * std::clamp(camDistance * 0.05f, 0.01f, 2.0f);
                    camDistance = glm::max(camDistance, 0.1f);
                    camVecAng.position = cameraFocus - camVecAng.forward * camDistance;
					editorCamera.setViewMatrix(camVecAng.toMatrix());
					disableGizmo = true;
                }
                if (ImGui::IsWindowFocused())
                {
                    if (ImGui::IsKeyDown(ImGuiKey_LeftAlt) && ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0.0f))
                    {
                        ImVec2 mouseDrag = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left, 0.0f);
                        camVecAng.yawDeg += mouseDrag.x * 0.15f;
                        camVecAng.pitchDeg -= mouseDrag.y * 0.15f;
                        camVecAng.pitchDeg = glm::clamp(camVecAng.pitchDeg, -89.0f, 89.0f);

						camVecAng.updateVectors();

						camVecAng.position = cameraFocus - camVecAng.forward * camDistance;
                        editorCamera.setViewMatrix(camVecAng.toMatrix());

                        disableGizmo = true;
                        //Utils::wrapCursor(pos, size);
                        ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
                    }
                    else if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle, 0.0f))
                    {
                        ImVec2 mouseDrag = ImGui::GetMouseDragDelta(ImGuiMouseButton_Middle, 0.0f);

						camVecAng.position += camVecAng.right * -mouseDrag.x * 0.01f;
						camVecAng.position += camVecAng.up * mouseDrag.y * 0.01f;
                        editorCamera.setViewMatrix(camVecAng.toMatrix());

                        disableGizmo = true;
                        //Utils::wrapCursor(pos, size);
                        ImGui::ResetMouseDragDelta(ImGuiMouseButton_Middle);
                    }
                }
            }


            ImGuizmo::SetOrthographic(false);
            ImGuizmo::SetDrawlist();
            ImGuizmo::SetRect(pos.x, pos.y, size.x, size.y);

            if (disableGizmo)
            {
                ImGuizmo::Enable(false);
            }
            else
            {
                ImGuizmo::Enable(true);
            }

            glm::mat4 cameraView = editorCamera.getViewMatrix();
            const ImVec2 viewManipSize = ImVec2(150, 150);
            ImVec2 viewManipPos = ImVec2(pos.x + size.x - viewManipSize.x, pos.y);

            ImVec2 dragDelta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left, 1.0f);
            if (ImGuizmo::IsUsingViewManipulate() && ImGui::IsMouseReleased(ImGuiMouseButton_Left)
                && dragDelta.x * dragDelta.x + dragDelta.y * dragDelta.y > 1.0f)
            {
                ImVec2 viewManipCenter = ImVec2(viewManipPos.x + viewManipSize.x / 2.0f, viewManipPos.y + viewManipSize.y / 2.0f);
                ImGui::TeleportMousePos(viewManipCenter);
            }

            ImGuizmo::ViewManipulate(glm::value_ptr(cameraView), camDistance,
                viewManipPos, viewManipSize, 0x10101010);

            if (ImGuizmo::IsUsingViewManipulate() && !disableGizmo)
            {
                if (ImGui::IsMouseDragging(ImGuiMouseButton_Left, 1.0f))
                {
                    ImGui::SetMouseCursor(ImGuiMouseCursor_None);
                }
                editorCamera.setViewMatrix(cameraView);
            }

            if (editor->selectedObject != (EntityID)-1 && editor->selectedObject != scene->getSceneRootEntity())
            {
                auto [width, height] = sceneFramebuffer->GetSizePair();
            
                auto entityMatrix = scene->getComponent<Transform>(editor->selectedObject).globalMatrix;
                auto& ts = scene->getTransformSystem();

				if (ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(editorCamera.getFrustum().getProjectionMatrix()),
                    gizmoOperation, gizmoMode, glm::value_ptr(entityMatrix)))
                {
                    ts.setGlobalMatrix(editor->selectedObject, entityMatrix);
                }
            }

        }

        ImGui::End();
        ImGui::PopStyleVar();
    }


    void SceneWindow::cameraControls(double scrollYOffset, float deltaTime)
    {
        if (scrollYOffset != 0.0f)
        {
            cameraSpeed += scrollYOffset * 0.1f;
            cameraSpeed = glm::clamp(cameraSpeed, 0.1f, 2.0f);
        }
        float scaledCamSpeed = cameraSpeed * deltaTime;

		auto camVecAng = editorCamera.getVectorsAndAngles();

        if (ImGui::IsKeyDown(ImGuiKey_W))
			camVecAng.position += camVecAng.forward * scaledCamSpeed;
        if (ImGui::IsKeyDown(ImGuiKey_S))
			camVecAng.position -= camVecAng.forward * scaledCamSpeed;
        if (ImGui::IsKeyDown(ImGuiKey_A))
			camVecAng.position -= camVecAng.right * scaledCamSpeed;
        if (ImGui::IsKeyDown(ImGuiKey_D))
			camVecAng.position += camVecAng.right * scaledCamSpeed;
        if (ImGui::IsKeyDown(ImGuiKey_Q))
			camVecAng.position -= camVecAng.up * scaledCamSpeed;
        if (ImGui::IsKeyDown(ImGuiKey_E))
			camVecAng.position += camVecAng.up * scaledCamSpeed;

        ImVec2 mouseDelta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right, 0.0f);

		camVecAng.yawDeg += mouseDelta.x * 0.1f;
		camVecAng.pitchDeg -= mouseDelta.y * 0.1f;

		camVecAng.pitchDeg = glm::clamp(camVecAng.pitchDeg, -89.0f, 89.0f);

		camVecAng.updateVectors();
        editorCamera.setViewMatrix(camVecAng.toMatrix());
    }
}