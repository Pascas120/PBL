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
#include "TextRenderer.h"


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

void end_frame();

constexpr int32_t WINDOW_WIDTH  = 1920;
constexpr int32_t WINDOW_HEIGHT = 1080;

GLFWwindow* window = nullptr;

// Change these to lower GL version like 4.5 if GL 4.6 can't be initialized on your machine
const     char*   glsl_version     = "#version 410";
constexpr int32_t GL_VERSION_MAJOR = 4;
constexpr int32_t GL_VERSION_MINOR = 1;

double scrollXOffset = 0.0f;
double scrollYOffset = 0.0f;

ImVec4 clear_color         = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
float deltaTime = 0.0f;
float lastFrame = 0.0f;


Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
//Shader ourShader;
//Shader hudShader;
//Shader textShader;
std::vector<Shader*> shaders;
std::vector<Model*> models;

//Model ourModel;

Scene scene;

EntityID player = (EntityID)-1;
EntityID selectedObject = (EntityID)-1;

std::unique_ptr<CustomFramebuffer> sceneFramebuffer;

//TransformSystem transformSystem = TransformSystem(&scene);
//RenderingSystem renderingSystem = RenderingSystem();
//EntityID  ent1;
//EntityID  ent2;
//EntityID  ent3;
//EntityID  ent4;
//
//float osc = 0;
//float width = 0.3;

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

    lastFrame = glfwGetTime();
    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        imgui_begin();
        imgui_render();

        update();
        render();      
        
        imgui_end();

        end_frame();
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    for (Shader* shader : shaders)
    {
        if (shader)
        {
            delete shader;
            shader = nullptr;
        }
    }

    for (Model* model : models)
    {
        if (model)
        {
            delete model;
            model = nullptr;
        }
    }

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
    glfwWindowHint(GLFW_MAXIMIZED, GL_TRUE);

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

    sceneFramebuffer = std::make_unique<CustomFramebuffer>(FramebufferConfig{ WINDOW_WIDTH, WINDOW_HEIGHT });


    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    shaders.emplace_back(
        new Shader("Basic", {
                { GL_VERTEX_SHADER, "res/shaders/basic.vert" },
                { GL_FRAGMENT_SHADER, "res/shaders/basic.frag" }
            }));
    shaders.emplace_back(
        new Shader("Flat", {
                { GL_VERTEX_SHADER, "res/shaders/flat.vert" },
                { GL_FRAGMENT_SHADER, "res/shaders/flat.frag" }
            }));
    shaders.emplace_back(
        new Shader("GUITest", {
                { GL_VERTEX_SHADER, "res/shaders/guitest.vert" },
                { GL_FRAGMENT_SHADER, "res/shaders/guitest.frag" }
            }));
	shaders.emplace_back(
		new Shader("HUD", {
				{ GL_VERTEX_SHADER, "res/shaders/HUD.vert" },
				{ GL_FRAGMENT_SHADER, "res/shaders/HUD.frag" }
			}));
	shaders.emplace_back(
		new Shader("Text", {
				{ GL_VERTEX_SHADER, "res/shaders/text.vert" },
				{ GL_FRAGMENT_SHADER, "res/shaders/text.frag" }
			}));

    models.emplace_back(new Model("res/models/nanosuit/nanosuit.obj"));
    models.emplace_back(new Model("res/models/dee/waddledee.obj"));
    models.emplace_back(new Model("res/models/grass_block/grass_block.obj"));
	models.emplace_back(new Model("res/models/untitled.fbx"));

    Model& ourModel = *models[0];
    Model& model2 = *models[1];
    Model& model3 = *models[2];

    scene = Scene();

    EntityID ent;
	ImageComponent* imageComponent;
	TextComponent* textComponent;
	ColliderComponent* colliderComponent;

	auto& ts = scene.GetTransformSystem();


    ent = scene.GetSceneRootEntity();
	scene.GetComponent<ObjectInfoComponent>(ent).name = "Root";


	ent = scene.CreateEntity();
    scene.GetComponent<ObjectInfoComponent>(ent).name = "Player";
    ts.scaleEntity(ent, glm::vec3(5.0f, 5.0f, 5.0f));

    scene.AddComponent<ModelComponent>(ent, { shaders[0], &model2 });

    colliderComponent = &scene.AddComponent<ColliderComponent>(ent, ColliderComponent(ColliderType::SPHERE));
    SphereCollider* sphereCollider = static_cast<SphereCollider*>(colliderComponent->GetColliderShape());
    sphereCollider->center = glm::vec3(-0.01f, 0.1f, 0.01f);
    sphereCollider->radius = 0.1f;


	ent = scene.CreateEntity();
	scene.GetComponent<ObjectInfoComponent>(ent).name = "Nanosuit";

	ts.scaleEntity(ent, glm::vec3(0.1f, 0.1f, 0.1f));
	ts.translateEntity(ent, glm::vec3(2.5f, 0.0f, 0.0f));

	scene.AddComponent<ModelComponent>(ent, { shaders[0], &ourModel });

	colliderComponent = &scene.AddComponent<ColliderComponent>(ent, ColliderComponent(ColliderType::BOX));
    BoxCollider* boxCollider = static_cast<BoxCollider*>(colliderComponent->GetColliderShape());
    boxCollider->center = glm::vec3(0.0f, 7.7f, 0.0f);
    boxCollider->halfSize = glm::vec3(4.0f, 7.7f, 1.778f);


	ent = scene.CreateEntity();
    scene.GetComponent<ObjectInfoComponent>(ent).name = "Floor";

	ts.scaleEntity(ent, glm::vec3(5.0, 0.1f, 5.0f));

	scene.AddComponent<ModelComponent>(ent, { shaders[0], &model3 });

	colliderComponent = &scene.AddComponent<ColliderComponent>(ent, ColliderComponent(ColliderType::BOX, true));


    std::pair<glm::vec3, glm::vec3> wallScalesAndTranslations[] = {
        { glm::vec3(5.0f, 1.0f, 1.0f), glm::vec3(0.0f, 1.0f, 6.0f) },
        { glm::vec3(5.0f, 1.0f, 1.0f), glm::vec3(0.0f, 1.0f, -6.0f) },
        { glm::vec3(1.0f, 1.0f, 5.0f), glm::vec3(6.0f, 1.0f, 0.0f) },
        { glm::vec3(1.0f, 1.0f, 5.0f), glm::vec3(-6.0f, 1.0f, 0.0f) },
    };

	for (int i = 1; i <= 4; ++i) {
		ent = scene.CreateEntity();
		scene.GetComponent<ObjectInfoComponent>(ent).name = "Wall " + std::to_string(i);

		ts.scaleEntity(ent, wallScalesAndTranslations[i].first);
		ts.translateEntity(ent, wallScalesAndTranslations[i].second);

		scene.AddComponent<ModelComponent>(ent, { shaders[0], &model3 });
		colliderComponent = &scene.AddComponent<ColliderComponent>(ent, ColliderComponent(ColliderType::BOX, true));
	}


	ent = scene.CreateEntity();
	scene.GetComponent<ObjectInfoComponent>(ent).name = "Wall 5";

	ts.rotateEntity(ent, glm::vec3(0.0f, 30.0f, 180.0f));
	ts.translateEntity(ent, glm::vec3(1.0f, 1.0f, 2.0f));
	ts.scaleEntity(ent, glm::vec3(0.5f, 2.0f, 0.5f));

	scene.AddComponent<ModelComponent>(ent, { shaders[0], &model3 });
	colliderComponent = &scene.AddComponent<ColliderComponent>(ent, ColliderComponent(ColliderType::BOX, true));


    ent = scene.CreateEntity();
    scene.GetComponent<ObjectInfoComponent>(ent).name = "Cloud";

	scene.AddComponent(ent, ImageComponent{ shaders[3], "res/textures/cloud.png" });
    ts.translateEntity(ent, glm::vec3(9*WINDOW_WIDTH/10, WINDOW_HEIGHT/10, 0.0f));
    ts.scaleEntity(ent, glm::vec3(250.0f));

    ent = scene.CreateEntity();
	scene.GetComponent<ObjectInfoComponent>(ent).name = "Text";

	scene.AddComponent(ent, TextComponent{ shaders[4], "foo", glm::vec4(1, 0, 0, 1), "text" });
    ts.translateEntity(ent, glm::vec3(1*WINDOW_WIDTH/10, WINDOW_HEIGHT/10, 0.0f));


//    scene.addChild(obj1);
//    obj1->addChild(obj2);
//    obj1->components.AddComponent<Transform>();
//    obj1->components.AddComponent<ModelComponent>(&ourModel);
//    obj2->components.AddComponent<Transform>();
//    obj2->components.AddComponent<ModelComponent>(&ourModel);
//
//    h1 = new HudElement(0.01 * WINDOW_WIDTH, 0.01 * WINDOW_HEIGHT, 0.3 * WINDOW_WIDTH, 0.05 * WINDOW_HEIGHT);
//    h1->setColor(glm::vec4(1,0,0,1));
//
//    h2 = new HudElement(0.8 * WINDOW_WIDTH, 0.0 * WINDOW_HEIGHT, 0.2 * WINDOW_WIDTH, 0.2 * WINDOW_HEIGHT);
//    h2->setTexture("../../res/textures/cloud.png");
//    hud.setRoot(h1);
//    h1->addChild(h2);

    glfwSetScrollCallback(window, [](GLFWwindow* window, double xoffset, double yoffset) {
        scrollXOffset += xoffset;
        scrollYOffset += yoffset;
    });

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
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Setup style
    ImGui::StyleColorsDark();

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
}

void update()
{

}

void render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	scene.GetRenderingSystem().drawScene(*sceneFramebuffer, camera);
	scene.GetRenderingSystem().drawHud(*sceneFramebuffer);
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
    /// Add new ImGui controls here
    // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).


    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
    {
        static float f = 0.0f;
        static int counter = 0;

        ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

        ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
        //ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state

        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
        ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

        if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
            counter++;
        ImGui::SameLine();
        ImGui::Text("counter = %d", counter);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();
    }

    // 3. Show another simple window.
    //if (show_another_window)
    //{
    //    ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
    //    ImGui::Text("Hello from another window!");
    //    if (ImGui::Button("Close Me"))
    //        show_another_window = false;
    //    ImGui::End();
    //}
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

