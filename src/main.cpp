// dear imgui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// If you are new to dear imgui, see examples/README.txt and documentation at the top of imgui.cpp.
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan graphics context creation, etc.)

#include "imgui.h"
#include "imgui_impl/imgui_impl_glfw.h"
#include "imgui_impl/imgui_impl_opengl3.h"
//#include <stdio.h>
#include "Shader.h"
#include "stb_image.h"
#include "Model.h"
#include "Camera.h"
#include "Scene.h"
#include "Transform.h"
#include "ModelComponent.h"

#define IMGUI_IMPL_OPENGL_LOADER_GLAD

#include <glad/glad.h>  // Initialize with gladLoadGL()
#include <GLFW/glfw3.h> // Include glfw3.h after our OpenGL definitions
#include <spdlog/spdlog.h>

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

bool init();
void init_imgui();

void input();
void update();
void render();

void imgui_begin();
void imgui_render();
void imgui_end();

void imgui_obj_info(GameObject* obj);
void imgui_transform(GameObject* obj);
void imgui_children(GameObject* obj);

void end_frame();

constexpr int32_t WINDOW_WIDTH  = 1920;
constexpr int32_t WINDOW_HEIGHT = 1080;

GLFWwindow* window = nullptr;

// Change these to lower GL version like 4.5 if GL 4.6 can't be initialized on your machine
const     char*   glsl_version     = "#version 410";
constexpr int32_t GL_VERSION_MAJOR = 4;
constexpr int32_t GL_VERSION_MINOR = 1;

bool show_wireframe = false;

ImVec4 clear_color         = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
float deltaTime = 0.0f;
float lastFrame = 0.0f;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
Shader ourShader;
Model ourModel;

Scene scene;
GameObject* obj1 = new GameObject();
GameObject* obj2 = new GameObject();
float osc = 0;

int main(int, char**)
{
    if (!init())
    {
        spdlog::error("Failed to initialize project!");
        return EXIT_FAILURE;
    }
    spdlog::info("Initialized project.");

    init_imgui();
    spdlog::info("Initialized ImGui.");

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        // Process I/O operations here
        input();

        // Update game objects' state here
        update();

        // OpenGL rendering code here
        render();

        // Draw ImGui
        imgui_begin();
        imgui_render(); // edit this function to add your own ImGui controls
        imgui_end(); // this call effectively renders ImGui

        // End frame and swap buffers (double buffering)
        end_frame();
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

bool init()
{
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) 
    {
        spdlog::error("Failed to initalize GLFW!");
        return false;
    }

    // GL 4.6 + GLSL 460
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GL_VERSION_MAJOR);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, GL_VERSION_MINOR);
    glfwWindowHint(GLFW_OPENGL_PROFILE,        GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only

    // Create window with graphics context
    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Dear ImGui GLFW+OpenGL4 example", NULL, NULL);
    if (window == NULL)
    {
        spdlog::error("Failed to create GLFW Window!");
        return false;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable VSync - fixes FPS at the refresh rate of your screen

    bool err = !gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    if (err)
    {
        spdlog::error("Failed to initialize OpenGL loader!");
        return false;
    }
//==============================================================================================
    glEnable(GL_DEPTH_TEST);
    stbi_set_flip_vertically_on_load(true);
    ourShader = Shader("../../res/shaders/basic.vert", "../../res/shaders/basic.frag");
    ourModel = Model("../../res/models/nanosuit/nanosuit.obj");

    scene.addChild(obj1);
    obj1->AddChild(obj2);
    obj1->components.AddComponent<Transform>();
    obj1->components.AddComponent<ModelComponent>(&ourModel);
    obj2->components.AddComponent<Transform>();
    obj2->components.AddComponent<ModelComponent>(&ourModel);

//==============================================================================================
    return true;
}

void init_imgui()
{
    // Setup Dear ImGui binding
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Setup style
    ImGui::StyleColorsDark();

}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset){
    camera.ProcessMouseScroll(yoffset);
}

void input()
{
    float currentFrame = glfwGetTime();
    static float lastFrame = 0.0f;
    float deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		camera.ProcessKeyboard(DOWN, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		camera.ProcessKeyboard(UP, deltaTime);

    // Add mouse controls if needed
    static double lastX = WINDOW_WIDTH/2.0f;
    static double lastY = WINDOW_HEIGHT/2.0f;
    static bool firstMouse = true;

    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
        camera.ProcessMouseMovement(xoffset, yoffset);

    float scrollOffset;

    glfwSetScrollCallback(window, scrollCallback);
}

void update()
{
    /*float currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    osc += 0.01f;
    obj1->components.GetComponent<Transform>()->setScale(glm::vec3(sin(osc)));
    obj1->MarkDirty();
    obj2->components.GetComponent<Transform>()->setTranslation(glm::vec3(sin(osc+0.5)*10));
    scene.Update();*/

	scene.Update();
}

void render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (!show_wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }else{
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    ourShader.use();

    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    ourShader.setMat4("projection", projection);
    ourShader.setMat4("view", view);

    scene.Draw(ourShader);
}

void imgui_begin()
{
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void imgui_render()
{
    static bool show_demo_window = true;

    if (show_demo_window)
        ImGui::ShowDemoWindow(&show_demo_window);


    {
        ImGui::Begin("Hello, world!");

        ImGui::Checkbox("Demo Window", &show_demo_window);
		ImGui::Checkbox("Wireframe", &show_wireframe);


        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

		ImGui::NewLine();

        imgui_children(scene.GetRoot());


        ImGui::End();
    }
}

void imgui_obj_info(GameObject* obj)
{
	ImGui::PushID(obj);

	if (ImGui::CollapsingHeader("[Object Name Here]", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Indent();
		imgui_transform(obj);
        ImGui::Unindent();

		if (obj->GetChildCount() > 0)
		{
			ImGui::Indent();
			ImGui::NewLine();

			imgui_children(obj);
			ImGui::Unindent();
		}
	}

	ImGui::PopID();
}

void imgui_transform(GameObject* obj)
{
	Transform* transform = obj->components.GetComponent<Transform>();

    if (!transform)
        return;

    ImGui::PushID(transform);

	if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
	{
		glm::vec3 translation = transform->getTranslation();
		glm::vec3 rotation = transform->getRotation();
		glm::vec3 scale = transform->getScale();
		if (ImGui::DragFloat3("Position", &translation[0], 0.1f))
		{
			transform->setTranslation(translation);
			obj->MarkDirty();
		}
		if (ImGui::DragFloat3("Rotation", &rotation[0], 0.1f))
		{
			transform->setRotation(rotation);
			obj->MarkDirty();
		}
		if (ImGui::DragFloat3("Scale", &scale[0], 0.1f))
		{
			transform->setScale(scale);
			obj->MarkDirty();
		}
	}

	ImGui::PopID();
}

void imgui_children(GameObject* obj)
{
	GameObject** children = obj->GetChildren();
	for (int i = 0; i < obj->GetChildCount(); i++)
	{
		if (children[i])
		{
			imgui_obj_info(children[i]);
		}
	}
}



void imgui_end()
{
    ImGui::Render();
    int display_w, display_h;
    glfwMakeContextCurrent(window);
    glfwGetFramebufferSize(window, &display_w, &display_h);

    glViewport(0, 0, display_w, display_h);
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void end_frame()
{
    glfwPollEvents();
    glfwMakeContextCurrent(window);
    glfwSwapBuffers(window);
}

