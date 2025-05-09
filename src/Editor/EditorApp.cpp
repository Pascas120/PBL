#include "EditorApp.h"

#include "editor_utils.h"

#include <glad/glad.h>  // Initialize with gladLoadGL()
#include <spdlog/spdlog.h>

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl/imgui_impl_glfw.h"
#include "imgui_impl/imgui_impl_opengl3.h"

#include "ImGuizmo.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#include "HierarchyWindow.h"
#include "InspectorWindow.h"
#include "ShaderWindow.h"

namespace Editor
{
    EditorApp::EditorApp() : Application()
    {
        initImGui();
        spdlog::info("Initialized ImGui.");

        sceneFramebuffer = std::make_unique<CustomFramebuffer>(FramebufferConfig{ WINDOW_WIDTH, WINDOW_HEIGHT });
		camera.Position = glm::vec3(0.0f, 0.0f, camDistance);
    }

    EditorApp::~EditorApp()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void EditorApp::run()
    {
        lastFrame = glfwGetTime();
        while (!glfwWindowShouldClose(window))
        {
            float currentFrame = glfwGetTime();
            deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;

            createImGuiDrawData();
			if (playMode)
			{
                update();
			}

            renderToWindow();
            renderImGui();
            endFrame();
        }
    }

    void EditorApp::initImGui()
    {
        // Setup Dear ImGui binding
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigWindowsMoveFromTitleBarOnly = true;


        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init(glsl_version);

        // Setup style
        ImGui::StyleColorsDark();


        // ImGuizmo

        gizmoOperation = ImGuizmo::OPERATION::UNIVERSAL;
		ImGuizmo::SetImGuiContext(ImGui::GetCurrentContext());
    }

    void EditorApp::createImGuiDrawData()
    {
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGuizmo::BeginFrame();


        static bool show_demo_window = false;
        static bool show_collisions = false;

        ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());

        if (ImGui::BeginMainMenuBar())
        {
			ImVec2 menuBarSize = ImGui::GetWindowSize();

            if (ImGui::BeginMenu("Settings"))
            {
                ImGui::MenuItem("Demo Window", NULL, &show_demo_window);
                ImGui::MenuItem("Collisions", NULL, &show_collisions);
                //ImGui::MenuItem("Wireframe", NULL, &show_wireframe);

                static bool enable_logging = true;
                if (ImGui::MenuItem("Logging", NULL, &enable_logging))
                {
                    if (enable_logging)
                    {
                        spdlog::set_level(spdlog::level::info);
                    }
                    else
                    {
                        spdlog::set_level(spdlog::level::off);
                    }
                }

                ImGui::EndMenu();
            }

            constexpr float playButtonWidth = 50;
            float playButtonPos = (menuBarSize.x - playButtonWidth) / 2;
			ImGui::SetCursorPosX(playButtonPos);
			/*if (ImGui::Button(playMode ? "Stop###playButton" : "Play###playButton", ImVec2(playButtonWidth, 0)))
			{
				playMode = !playMode;
			}*/
			ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.5f, 0.5f));
            if (ImGui::Selectable(playMode ? "Stop###playButton" : "Play###playButton", playMode,
                ImGuiSelectableFlags_None, ImVec2(playButtonWidth, 0)))
            {
				playMode = !playMode;
            }
			ImGui::PopStyleVar();

			static const ImVec2 resButtonSize = ImVec2(200, 0);
			static const float resButtonRightOffset = 100.0f;
			ImGui::SetCursorPosX(menuBarSize.x - resButtonRightOffset - resButtonSize.x);
            if (ImGui::Button("Open resource folder", resButtonSize))
            {
                Utils::openFolder("res");
            }

            std::string fpsText = std::format("{:.1f} FPS", ImGui::GetIO().Framerate);
			float fpsTextWidth = ImGui::CalcTextSize(fpsText.c_str()).x;
			ImGui::SetCursorPosX(menuBarSize.x - (resButtonRightOffset + fpsTextWidth) / 2.0f);
            ImGui::Text(fpsText.c_str());
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                ImGui::Text("Average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
                ImGui::EndTooltip();
            }

            ImGui::EndMainMenuBar();
        }

        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);


        imguiScene();
		EditorContext context{ this, scene, shaders, models, camera };

		hierarchyWindow->draw(context);
		inspectorWindow->draw(context);
        shaderWindow->draw(context);

        if (ImGui::BeginViewportSideBar("Log sidebar", ImGui::GetMainViewport(), ImGuiDir_Down,
            ImGui::GetFrameHeight(), ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar))
        {
            if (ImGui::BeginMenuBar())
            {
                ImGui::Text("Log?");
                ImGui::EndMenuBar();
            }
        }

        ImGui::End();

        ImGui::Render();
    }

    void EditorApp::renderImGui()
    {
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    static void wrapCursor(ImVec2 pos, ImVec2 size)
    {
        ImVec2 mousePos = ImGui::GetMousePos();

        float relX = mousePos.x - pos.x;
        float relY = mousePos.y - pos.y;

        if (relX < 0.0f || relX >= size.x || relY < 0.0f || relY >= size.y)
        {
            relX = fmodf(relX + size.x, size.x);
            relY = fmodf(relY + size.y, size.y);


            mousePos.x = pos.x + relX;
            mousePos.y = pos.y + relY;

            ImGui::TeleportMousePos(mousePos);
        }
    }


    void EditorApp::imguiScene()
    {
        static ImVec2 lastSize = ImVec2(0, 0);

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
            if (ImGui::IsWindowFocused() && ImGuizmo::IsUsingAny() || ImGuizmo::IsUsingViewManipulate())
            {
				wrapCursor(pos, size);
            }

            /*if (!ImGui::IsAnyItemActive())
            {
                if (ImGui::IsWindowFocused())
                {
                    input();
                }
            }*/

            if (size.x != lastSize.x || size.y != lastSize.y)
            {
                sceneFramebuffer->Resize(size.x, size.y);
                lastSize = size;
            }

            render(*sceneFramebuffer);

            GLuint texture = sceneFramebuffer->GetColorTexture();
            ImGui::Image(texture, size, ImVec2(0, 1), ImVec2(1, 0));


			bool disableGizmo = false;

            glm::vec3 cameraFocus = camera.Position + camera.Front * camDistance;

            if (ImGui::IsWindowFocused() && ImGui::IsMouseDown(ImGuiMouseButton_Right))
            {
				cameraControls();
                wrapCursor(pos, size);
				disableGizmo = true;
                ImGui::ResetMouseDragDelta(ImGuiMouseButton_Right);
            }
            else if (ImGui::IsWindowHovered() && !Utils::isActiveInAnotherWindow())
            {
                if (scrollYOffset != 0.0f)
                {
                    camDistance -= scrollYOffset * std::clamp(camDistance * 0.05f, 0.01f, 2.0f);
                    camDistance = glm::max(camDistance, 0.1f);
                    camera.Position = cameraFocus - camera.Front * camDistance;
                }
                if (ImGui::IsWindowFocused())
                {
                    if (ImGui::IsKeyDown(ImGuiKey_LeftAlt) && ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0.0f))
                    {
                        ImVec2 mouseDrag = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left, 0.0f);
                        camera.Yaw += mouseDrag.x * 0.15f;
                        camera.Pitch -= mouseDrag.y * 0.15f;
                        camera.Pitch = glm::clamp(camera.Pitch, -89.0f, 89.0f);

                        camera.updateCameraVectors();

                        camera.Position = cameraFocus - camera.Front * camDistance;

                        disableGizmo = true;
                        wrapCursor(pos, size);
                        ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
                    }
                    else if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle, 0.0f))
                    {
                        ImVec2 mouseDrag = ImGui::GetMouseDragDelta(ImGuiMouseButton_Middle, 0.0f);
                        camera.Position += camera.Right * -mouseDrag.x * 0.01f;
                        camera.Position += camera.Up * mouseDrag.y * 0.01f;

                        disableGizmo = true;
                        wrapCursor(pos, size);
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

            glm::mat4 cameraView = camera.getViewMatrix();
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
                camera.setViewMatrix(cameraView);
			}

			if (selectedObject != (EntityID)-1 && selectedObject != scene->getSceneRootEntity())
			{
                auto [width, height] = sceneFramebuffer->GetSizePair();
                glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)width / (float)height, 0.1f, 100.0f);

                auto entityMatrix = scene->getComponent<Transform>(selectedObject).globalMatrix;
				auto& ts = scene->getTransformSystem();

                if (ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(projection),
                    gizmoOperation, ImGuizmo::MODE::LOCAL, glm::value_ptr(entityMatrix)))
                {
					ts.setGlobalMatrix(selectedObject, entityMatrix);
                }
			}

        }

        ImGui::End();
        ImGui::PopStyleVar();
    }


    void EditorApp::cameraControls()
    {
        bool firstMouse = ImGui::IsMouseClicked(ImGuiMouseButton_Right);
        if (scrollYOffset != 0.0f)
        {
            cameraSpeed += scrollYOffset * 0.1f;
            cameraSpeed = glm::clamp(cameraSpeed, 0.1f, 2.0f);
        }
        float scaledCamSpeed = cameraSpeed * deltaTime;

		if (ImGui::IsKeyDown(ImGuiKey_W))
			camera.processKeyboard(FORWARD, scaledCamSpeed);
		if (ImGui::IsKeyDown(ImGuiKey_S))
			camera.processKeyboard(BACKWARD, scaledCamSpeed);
		if (ImGui::IsKeyDown(ImGuiKey_A))
			camera.processKeyboard(LEFT, scaledCamSpeed);
		if (ImGui::IsKeyDown(ImGuiKey_D))
			camera.processKeyboard(RIGHT, scaledCamSpeed);
		if (ImGui::IsKeyDown(ImGuiKey_Q))
			camera.processKeyboard(DOWN, scaledCamSpeed);
		if (ImGui::IsKeyDown(ImGuiKey_E))
			camera.processKeyboard(UP, scaledCamSpeed);

		ImVec2 mouseDelta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right, 0.0f);

		camera.processMouseMovement(mouseDelta.x, -mouseDelta.y);
    }

}

