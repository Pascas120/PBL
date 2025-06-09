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
#include <fstream>

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
    	spdlog::info("dupa");
        initImGui();
        spdlog::info("Initialized ImGui.");

        assert(NFD_Init() == NFD_OKAY && "Failed to initialize NFD!");
        spdlog::info("Initialized NFD");

    	hierarchyWindow = std::make_unique<HierarchyWindow>(this);
    	inspectorWindow = std::make_unique<InspectorWindow>(this);
    	shaderWindow = std::make_unique<ShaderWindow>(this);

    	sceneWindow = std::make_unique<SceneWindow>(this);
    	gameWindow = std::make_unique<GameWindow>(this);


        std::vector<Shader*> editorShaderVec;
        Serialization::loadShaderList("res/editor/editorShaderList.json", editorShaderVec);
        for (Shader* shader : editorShaderVec) {
            editorShaders[shader->getName()] = shader;
            GLint numUniformBlocks;
            glGetProgramiv(shader->ID, GL_ACTIVE_UNIFORM_BLOCKS, &numUniformBlocks);

            for (int i = 0; i < numUniformBlocks; ++i)
            {
                GLchar blockName[256];
                glGetActiveUniformBlockName(shader->ID, i, sizeof(blockName), nullptr, blockName);
                auto it = uniformBlockMap.find(blockName);
                if (it != uniformBlockMap.end())
                {
                    shader->use();
                    if (it->second->isInitialized())
                    {
                        it->second->init(shader->ID);
                    }
                    else
                    {
                        it->second->bindToShader(shader->ID);
                    }
                }

            }
        }
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

            /*if (ImGui::BeginMenu("Prefabs"))
            {
                ImGui::OpenPopup("prefabWindow");

				ImGui::EndMenu();
            }*/

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

			static const ImVec2 prefabButtonSize = ImVec2(100, 0);
			static const ImVec2 resButtonSize = ImVec2(200, 0);
			static const float resButtonRightOffset = 100.0f;
			ImGui::SetCursorPosX(menuBarSize.x - resButtonRightOffset - resButtonSize.x);
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() - prefabButtonSize.x);

			if (ImGui::Button("Prefabs", prefabButtonSize))
			{
                ImGui::OpenPopup("prefabWindow");
			}
            prefabWindow();

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

			auto breadControllers = scene->getStorage<BreadController>();
            if (breadControllers)
            {
				for (int i = 0; i < breadControllers->getQuantity(); i++)
				{
					auto& breadController = breadControllers->components[i];
					breadController.startScale = scene->getComponent<Transform>(breadController.id).scale;
				}
            }
			auto butterHealthComponents = scene->getStorage<ButterHealthComponent>();
            if (butterHealthComponents)
            {
                for (int i = 0; i < butterHealthComponents->getQuantity(); i++)
                {
                    auto& bh = butterHealthComponents->components[i];
                    bh.startScale = scene->getComponent<Transform>(bh.id).scale;
                }
            }

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
                setupEvents();
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
			auto pathResult = Utils::openSaveDialog({ {"Scene JSON", ".2json"} }, "res/scenes");
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
		auto pathResult = Utils::openLoadDialog({ {"Scene JSON", "json"} }, "res/scenes");
		if (!pathResult)
		{
			return;
		}
		selectedObject = (EntityID)-1;
		setPlayMode(PlayMode::STOP);

		scene = std::make_shared<Scene>(this);
		std::string path = *pathResult;
		Serialization::loadScene(path, *scene, { shaders, models, true });
		scenePath = path;


        setupEvents();
	}



    Shader* EditorApp::getEditorShader(const std::string& name) const
    {
        auto it = editorShaders.find(name);
        if (it != editorShaders.end())
        {
            return it->second;
        }

        return nullptr;
    }

    void EditorApp::prefabWindow()
    {

		if (!ImGui::BeginPopup("prefabWindow", ImGuiWindowFlags_NoDocking))
			return;

		static std::string prefabName;
        char prefabNameBuffer[128];
		std::strncpy(prefabNameBuffer, prefabName.c_str(), sizeof(prefabNameBuffer));
        if (ImGui::InputText("Prefab Name", prefabNameBuffer, sizeof(prefabNameBuffer)))
        {
			prefabName = std::string(prefabNameBuffer);
        }

        if (ImGui::BeginListBox("##prefabList"))
        {
			for (const auto& [name, prefab] : prefabs)
			{
				bool isSelected = (name == prefabName);
				if (ImGui::Selectable(name.c_str(), isSelected))
				{
					prefabName = name;
				}
			}
			ImGui::EndListBox();
        }

		ImGui::BeginDisabled(selectedObject == (EntityID)-1);

            auto it = prefabs.find(prefabName);
		    bool prefabExists = it != prefabs.end();
		    ImGui::BeginDisabled(!prefabExists);
		    if (ImGui::Button("Instantiate"))
		    {
			    auto instantiatedEntities = Serialization::deserializeObjects(it->second, *scene, selectedObject, { shaders, models, false });
			    /*if (!instantiatedEntities.empty())
			    {
				    selectedObject = instantiatedEntities[0];
			    }*/
		    }
		    ImGui::EndDisabled();

		    ImGui::SameLine();
            ImGui::BeginDisabled(!prefabExists && prefabName.empty() || selectedObject == scene->getSceneRootEntity());
            if (ImGui::Button(prefabExists ? "Update" : "Create"))
            {
                prefabs[prefabName] = Serialization::serializeObjects({ selectedObject }, *scene);

				// TODO?: osobne pliki dla każdego prefabu
				std::ofstream prefabFile("res/prefabs.json");
                if (prefabFile.is_open())
                {
                    json prefabListJson;
                    prefabListJson["prefabs"] = json::array();
                    for (const auto& [name, prefab] : prefabs)
                    {
                        json prefabJson;
                        prefabJson["name"] = name;
                        prefabJson["data"] = prefab;
                        prefabListJson["prefabs"].push_back(prefabJson);
                    }

					prefabFile << std::setw(4) << prefabListJson << std::endl;
					prefabFile.close();
				}
				else
				{
					spdlog::error("Failed to open prefab file for writing.");
                }
            }
		    ImGui::EndDisabled();

		ImGui::EndDisabled();

        ImGui::EndPopup();

    }

}

