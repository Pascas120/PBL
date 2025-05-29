#include "EditorApp.h"

#include "Utils/editor_utils.h"

#include <glad/glad.h>  // Initialize with gladLoadGL()
#include <spdlog/spdlog.h>

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl/imgui_impl_glfw.h"
#include "imgui_impl/imgui_impl_opengl3.h"

#include "ImGuizmo.h"

#include <nfd.h>
#include <filesystem>
#include "Serialization.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#include "HierarchyWindow.h"
#include "InspectorWindow.h"
#include "ShaderWindow.h"
#include "SceneWindow.h"
#include "GameWindow.h"

namespace Editor
{
    EditorApp::EditorApp() : Application()
    {
        initImGui();
        spdlog::info("Initialized ImGui.");

        assert(NFD_Init() == NFD_OKAY && "Failed to initialize NFD!");
        spdlog::info("Initialized NFD");

    	hierarchyWindow = std::make_unique<HierarchyWindow>(this);
    	inspectorWindow = std::make_unique<InspectorWindow>(this);
    	shaderWindow = std::make_unique<ShaderWindow>(this);

    	sceneWindow = std::make_unique<SceneWindow>(this);
    	gameWindow = std::make_unique<GameWindow>(this);
    }

    EditorApp::~EditorApp()
    {
		NFD_Quit();

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
			if (playMode == PlayMode::PLAY)
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

        ImGuiInputFlags shortcutFlags = ImGuiInputFlags_RouteGlobal;
        if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_S, shortcutFlags))
        {
			saveScene(scenePath);
        }
        else if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_S, shortcutFlags))
		{
			saveScene();
		}
		else if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_L, shortcutFlags))
		{
			loadScene();
		}


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

			if (ImGui::BeginMenu("File"))
			{
				ImGui::BeginDisabled(scenePath.empty());
                if (ImGui::MenuItem("Save Scene", "Ctrl+S"))
                {
					saveScene(scenePath);
                }
				ImGui::EndDisabled();
				if (ImGui::MenuItem("Save Scene As", "Ctrl+Shift+S"))
				{
					saveScene();
				}

				if (ImGui::MenuItem("Load Scene", "Ctrl+L"))
				{
					loadScene();
				}

				ImGui::EndMenu();
			}

            constexpr float playButtonWidth = 70;
            float playButtonPos = (menuBarSize.x - playButtonWidth * 2) / 2;
			ImGui::SetCursorPosX(playButtonPos);

			ImGui::PushStyleColor(ImGuiCol_Button, 
				playMode != PlayMode::STOP ? ImGui::GetStyleColorVec4(ImGuiCol_Button) 
                : ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
            if (ImGui::Button(playMode != PlayMode::STOP ? "Stop###playButton" : "Play###playButton",
                ImVec2(playButtonWidth, 0)))
            {
				setPlayMode(playMode == PlayMode::STOP ? PlayMode::PLAY : PlayMode::STOP);
            }
			ImGui::PopStyleColor();

			ImGui::SetCursorPosX(playButtonPos + playButtonWidth);
            ImGui::PushStyleColor(ImGuiCol_Button,
                playMode == PlayMode::PAUSE ? ImGui::GetStyleColorVec4(ImGuiCol_Button)
                : ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
			ImGui::BeginDisabled(playMode == PlayMode::STOP);
            if (ImGui::Button(playMode == PlayMode::PAUSE ? "Resume###pauseButton" : "Pause###pauseButton",
                    ImVec2(playButtonWidth, 0)))
            {
				setPlayMode(playMode == PlayMode::PAUSE ? PlayMode::PLAY : PlayMode::PAUSE);
            }
			ImGui::EndDisabled();
            ImGui::PopStyleColor();


			static const ImVec2 resButtonSize = ImVec2(200, 0);
			static const float resButtonRightOffset = 100.0f;
			ImGui::SetCursorPosX(menuBarSize.x - resButtonRightOffset - resButtonSize.x);
            if (ImGui::Button("Open resource folder", resButtonSize))
            {
                Utils::openFolder("res");
            }

            std::string fpsText = fmt::format("{:.1f} FPS", ImGui::GetIO().Framerate);
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


        //imguiScene();
		EditorContext context{ scene, shaders, models };

		hierarchyWindow->draw(context);
		inspectorWindow->draw(context);
        shaderWindow->draw(context);
		sceneWindow->draw(context);
		gameWindow->draw(context);

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

    void EditorApp::setPlayMode(PlayMode mode)
    {
		if (playMode == mode)
			return;

		if (mode == PlayMode::PLAY && playMode == PlayMode::STOP)
        {
            sceneBackup = std::make_shared<Scene>(*scene);
			ImGui::SetWindowFocus("Game");
			scene->getRenderingSystem().buildTree();
        }
		else if (mode == PlayMode::STOP)
		{
			if (sceneBackup)
            {
                if (selectedObject != (EntityID)-1)
                {
                    std::string& selectedObjectUuid = scene->getComponent<ObjectInfoComponent>(selectedObject).uuid;
                    if (!sceneBackup->hasEntity(selectedObject) || sceneBackup->getComponent<ObjectInfoComponent>(selectedObject).uuid != selectedObjectUuid)
                    {
                        selectedObject = (EntityID)-1;
                    }
                }

                scene = std::make_shared<Scene>(*sceneBackup);
                sceneBackup = nullptr;
            }
			ImGui::SetWindowFocus("Scene");
		}
        playMode = mode;
    }


	void EditorApp::saveScene(std::string path)
	{
		if (path.empty())
		{
			auto pathResult = Utils::openSaveDialog({ {"Scene JSON", "scene.json"} }, "res/scenes");
			if (!pathResult)
			{
				return;
			}
			path = *pathResult;
		}
		Serialization::saveScene(path, *scene);
		scenePath = path;
	}

	void EditorApp::loadScene()
	{
		auto pathResult = Utils::openLoadDialog({ {"Scene JSON", "scene.json"} }, "res/scenes");
		if (!pathResult)
		{
			return;
		}
		selectedObject = (EntityID)-1;
		setPlayMode(PlayMode::STOP);

		scene = std::make_shared<Scene>();
		std::string path = *pathResult;
		Serialization::loadScene(path, *scene, { shaders, models, true });
		scenePath = path;


        EventSystem& eventSystem = scene->getEventSystem();
        eventSystem.registerListener<CollisionEvent>([&](const Event& e) {
            const auto& event = static_cast<const CollisionEvent&>(e);
            if (!event.isColliding) return;

            bool aIsPlayer = (event.objectA == player);
            bool bIsPlayer = (event.objectB == player);

            bool aIsFly = scene->hasComponent<FlyAIComponent>(event.objectA);
            bool bIsFly = scene->hasComponent<FlyAIComponent>(event.objectB);


            if ((aIsPlayer && bIsFly) || (bIsPlayer && aIsFly))
            {
                spdlog::info("mucha uderzyla!({} vs {})", event.objectA, event.objectB);
                FlyAIComponent& fly = scene->getComponent<FlyAIComponent>(aIsFly ? event.objectA : event.objectB);
                fly.diveCooldownTimer = fly.diveCooldownTime;
                fly.state = fly.Returning;
            }
            });

        eventSystem.registerListener<CollisionEvent>([&](const Event& e) {
            const auto& event = static_cast<const CollisionEvent&>(e);
            if (!event.isColliding) return;

			VelocityComponent* componentA = scene->hasComponent<VelocityComponent>(event.objectA) ?
				&scene->getComponent<VelocityComponent>(event.objectA) : nullptr;
			VelocityComponent* componentB = scene->hasComponent<VelocityComponent>(event.objectB) ?
				&scene->getComponent<VelocityComponent>(event.objectB) : nullptr;

            if (abs(event.separationVector.y) > 0.01f)
            {
                if (componentA && componentA->useGravity)
                {
                    componentA->velocity.y = 0.0f;
                }
                if (componentB && componentB->useGravity)
                {
                    componentB->velocity.y = 0.0f;
                }
            }
            });
	}

}

