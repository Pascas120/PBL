#include "SceneWindow.h" 
#include "editor_utils.h"

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
	SceneWindow::SceneWindow()
	{
		sceneFramebuffer = std::make_unique<CustomFramebuffer>(FramebufferConfig{ 0, 0 });
		editorCamera.Position = glm::vec3(0.0f, 0.0f, camDistance);
		gizmoOperation = ImGuizmo::OPERATION::UNIVERSAL;
	}

    void SceneWindow::draw(const EditorContext& context)
    {
		drawWindow(context);
    }


    void SceneWindow::drawWindow(const EditorContext& context)
    {
		auto& editor = context.editor;
		auto& scene = context.scene;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(650, 400), ImGuiCond_FirstUseEver);

        if (ImGui::Begin("Scene"));
        {
            /*if (ImGui::BeginMenuBar())
            {
                if (ImGui::RadioButton("Translate", gizmoOperation == ImGuizmo::TRANSLATE))
                    gizmoOperation = ImGuizmo::TRANSLATE;
                ImGui::SameLine();

                if (ImGui::RadioButton("Rotate", gizmoOperation == ImGuizmo::ROTATE))
                    gizmoOperation = ImGuizmo::ROTATE;
                ImGui::SameLine();

                if (ImGui::RadioButton("Scale", gizmoOperation == ImGuizmo::SCALE))
                    gizmoOperation = ImGuizmo::SCALE;
                ImGui::SameLine();

                if (ImGui::RadioButton("Universal", gizmoOperation == ImGuizmo::UNIVERSAL))
                    gizmoOperation = ImGuizmo::UNIVERSAL;

                ImGui::EndMenuBar();
            }*/

            ImVec2 pos = ImGui::GetCursorScreenPos();
            ImVec2 size = ImGui::GetContentRegionAvail();


            if (ImGui::IsWindowHovered() &&
                (ImGui::IsMouseClicked(ImGuiMouseButton_Right) || ImGui::IsMouseClicked(ImGuiMouseButton_Middle)))
            {
                ImGui::SetWindowFocus();
            }

            // TODO: fix (doesn't work with imguizmo)
            if (ImGui::IsWindowFocused() && (ImGuizmo::IsUsingAny() || ImGuizmo::IsUsingViewManipulate()))
            {
                Utils::wrapCursor(pos, size);
            }

            if (size.x != lastSize.x || size.y != lastSize.y)
            {
                sceneFramebuffer->Resize(size.x, size.y);
                lastSize = size;
            }

            editor->render(editorCamera, *sceneFramebuffer);

            GLuint texture = sceneFramebuffer->GetColorTexture();
            ImGui::Image(texture, size, ImVec2(0, 1), ImVec2(1, 0));


            bool disableGizmo = false;

            glm::vec3 cameraFocus = editorCamera.Position + editorCamera.Front * camDistance;
            double scrollYOffset = editor->getScrollOffset().second;

            if (ImGui::IsWindowFocused() && ImGui::IsMouseDown(ImGuiMouseButton_Right))
            {
				cameraControls(scrollYOffset, editor->getDeltaTime());
                Utils::wrapCursor(pos, size);
                disableGizmo = true;
                ImGui::ResetMouseDragDelta(ImGuiMouseButton_Right);
            }
            else if (ImGui::IsWindowHovered() && !Utils::isActiveInAnotherWindow())
            {
                if (scrollYOffset != 0.0f)
                {
                    camDistance -= scrollYOffset * std::clamp(camDistance * 0.05f, 0.01f, 2.0f);
                    camDistance = glm::max(camDistance, 0.1f);
                    editorCamera.Position = cameraFocus - editorCamera.Front * camDistance;
                }
                if (ImGui::IsWindowFocused())
                {
                    if (ImGui::IsKeyDown(ImGuiKey_LeftAlt) && ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0.0f))
                    {
                        ImVec2 mouseDrag = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left, 0.0f);
                        editorCamera.Yaw += mouseDrag.x * 0.15f;
                        editorCamera.Pitch -= mouseDrag.y * 0.15f;
                        editorCamera.Pitch = glm::clamp(editorCamera.Pitch, -89.0f, 89.0f);

                        editorCamera.updateCameraVectors();

                        editorCamera.Position = cameraFocus - editorCamera.Front * camDistance;

                        disableGizmo = true;
                        Utils::wrapCursor(pos, size);
                        ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
                    }
                    else if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle, 0.0f))
                    {
                        ImVec2 mouseDrag = ImGui::GetMouseDragDelta(ImGuiMouseButton_Middle, 0.0f);
                        editorCamera.Position += editorCamera.Right * -mouseDrag.x * 0.01f;
                        editorCamera.Position += editorCamera.Up * mouseDrag.y * 0.01f;

                        disableGizmo = true;
                        Utils::wrapCursor(pos, size);
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
                glm::mat4 projection = glm::perspective(glm::radians(editorCamera.Zoom), (float)width / (float)height, 0.1f, 100.0f);

                auto entityMatrix = scene->getComponent<Transform>(editor->selectedObject).globalMatrix;
                auto& ts = scene->getTransformSystem();

                if (ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(projection),
                    gizmoOperation, ImGuizmo::MODE::LOCAL, glm::value_ptr(entityMatrix)))
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
        bool firstMouse = ImGui::IsMouseClicked(ImGuiMouseButton_Right);
        if (scrollYOffset != 0.0f)
        {
            cameraSpeed += scrollYOffset * 0.1f;
            cameraSpeed = glm::clamp(cameraSpeed, 0.1f, 2.0f);
        }
        float scaledCamSpeed = cameraSpeed * deltaTime;

        if (ImGui::IsKeyDown(ImGuiKey_W))
            editorCamera.processKeyboard(FORWARD, scaledCamSpeed);
        if (ImGui::IsKeyDown(ImGuiKey_S))
            editorCamera.processKeyboard(BACKWARD, scaledCamSpeed);
        if (ImGui::IsKeyDown(ImGuiKey_A))
            editorCamera.processKeyboard(LEFT, scaledCamSpeed);
        if (ImGui::IsKeyDown(ImGuiKey_D))
            editorCamera.processKeyboard(RIGHT, scaledCamSpeed);
        if (ImGui::IsKeyDown(ImGuiKey_Q))
            editorCamera.processKeyboard(DOWN, scaledCamSpeed);
        if (ImGui::IsKeyDown(ImGuiKey_E))
            editorCamera.processKeyboard(UP, scaledCamSpeed);

        ImVec2 mouseDelta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right, 0.0f);

        editorCamera.processMouseMovement(mouseDelta.x, -mouseDelta.y);
    }
}