#include "EditorApp.h"

#include <glad/glad.h>  // Initialize with gladLoadGL()
#include <spdlog/spdlog.h>

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl/imgui_impl_glfw.h"
#include "imgui_impl/imgui_impl_opengl3.h"

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

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init(glsl_version);

        // Setup style
        ImGui::StyleColorsDark();

    }

    void EditorApp::createImGuiDrawData()
    {
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();


        static bool show_demo_window = false;
        static bool show_collisions = false;

        ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());

        if (ImGui::BeginMainMenuBar())
        {
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

            std::string fpsText = std::format("{:.1f} FPS", ImGui::GetIO().Framerate);
            ImGui::SetCursorPosX(ImGui::GetWindowSize().x - ImGui::CalcTextSize(fpsText.c_str()).x - 10.0f);
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
		EditorContext context{ this, scene, shaders, models };

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


    void EditorApp::imguiScene()
    {
        static ImVec2 lastSize = ImVec2(0, 0);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(650, 400), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Scene"))
        {
            //ImVec2 mousePos = ImGui::GetMousePos();
            //ImVec2 imguiCursorPos = ImGui::GetCursorScreenPos();
            //ImVec2 relativeCursorPos = ImVec2(mousePos.x - imguiCursorPos.x, mousePos.y - imguiCursorPos.y);

            if (!ImGui::IsAnyItemActive())
            {
                if (ImGui::IsWindowFocused())
                {
                    input();
                }
                /*if (ImGui::IsWindowHovered())
                {
                    input();
                }*/
            }

            ImVec2 size = ImGui::GetContentRegionAvail();
            if (size.x != lastSize.x || size.y != lastSize.y)
            {
                sceneFramebuffer->Resize(size.x, size.y);
                lastSize = size;
            }

            render(*sceneFramebuffer);

            GLuint texture = sceneFramebuffer->GetColorTexture();
            ImGui::Image(texture, size, ImVec2(0, 1), ImVec2(1, 0));
        }

        ImGui::End();
        ImGui::PopStyleVar();
    }
}

