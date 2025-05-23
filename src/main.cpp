// dear imgui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// If you are new to dear imgui, see examples/README.txt and documentation at the top of imgui.cpp.
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan graphics context creation, etc.)
#include "imgui.h"
#include "imgui_internal.h"
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
void render(const Framebuffer& framebuffer = DefaultFramebuffer::GetInstance());

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

bool show_wireframe = false;

ImVec4 clear_color         = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
float deltaTime = 0.0f;
float lastFrame = 0.0f;


Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

std::vector<Shader*> shaders;
std::vector<Model*> models;


Scene scene;

EntityID player = (EntityID)-1;
EntityID selectedObject = (EntityID)-1;

std::unique_ptr<CustomFramebuffer> sceneFramebuffer;


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
        render(*sceneFramebuffer);
        
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
    glEnable(GL_CULL_FACE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    shaders.emplace_back(
        new Shader("Basic", {
                { GL_VERTEX_SHADER, "res/shaders/basic.vert" },
                { GL_FRAGMENT_SHADER, "res/shaders/basic.frag" }
            }));
    shaders.emplace_back(
            new Shader("Material", {
                    { GL_VERTEX_SHADER, "res/shaders/basic.vert" },
                    { GL_FRAGMENT_SHADER, "res/shaders/material.frag" }
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
    shaders.emplace_back(
    new Shader("Anim", {
            { GL_VERTEX_SHADER, "res/shaders/anim.vert" },
            { GL_FRAGMENT_SHADER, "res/shaders/basic.frag" }
        }));

    models.emplace_back(new Model("res/models/nanosuit/nanosuit.obj"));
    models.emplace_back(new Model("res/models/dee/waddledee.obj"));
    models.emplace_back(new Model("res/models/grass_block/grass_block.obj"));
	models.emplace_back(new Model("res/models/untitled.fbx"));
    models.emplace_back(new Model("res/models/maslpo.fbx"));

    Model& ourModel = *models[0];
    Model& model2 = *models[1];
    Model& model3 = *models[2];
    Model& model4 = *models[4];

    EntityID ent;
	ImageComponent* imageComponent;
	TextComponent* textComponent;
	ColliderComponent* colliderComponent;

	auto& ts = scene.getTransformSystem();


    ent = scene.getSceneRootEntity();
	scene.getComponent<ObjectInfoComponent>(ent).name = "Root";

	ent = player = scene.createEntity();
    scene.getComponent<ObjectInfoComponent>(ent).name = "Player";
    ts.scaleEntity(ent, glm::vec3(5.0f, 5.0f, 5.0f));

    scene.addComponent<ModelComponent>(ent, { shaders[0], &model2 });

    colliderComponent = &scene.addComponent<ColliderComponent>(ent, ColliderComponent(ColliderType::SPHERE));
    SphereCollider* sphereCollider = static_cast<SphereCollider*>(colliderComponent->GetColliderShape());
    sphereCollider->center = glm::vec3(-0.01f, 0.1f, 0.01f);
    sphereCollider->radius = 0.1f;

    scene.addComponent(ent, BoundingVolumeComponent(std::make_unique<AABBBV>(model2.calculateBoundingBox())));


	ent = scene.createEntity();
	scene.getComponent<ObjectInfoComponent>(ent).name = "Nanosuit";

	ts.scaleEntity(ent, glm::vec3(0.1f, 0.1f, 0.1f));
	ts.translateEntity(ent, glm::vec3(2.5f, 0.0f, 0.0f));

	scene.addComponent<ModelComponent>(ent, { shaders[0], &ourModel });

	colliderComponent = &scene.addComponent<ColliderComponent>(ent, ColliderComponent(ColliderType::BOX));
    BoxCollider* boxCollider = static_cast<BoxCollider*>(colliderComponent->GetColliderShape());
    boxCollider->center = glm::vec3(0.0f, 7.7f, 0.0f);
    boxCollider->halfSize = glm::vec3(4.0f, 7.7f, 1.778f);

    AABBBV boundingBox = ourModel.calculateBoundingBox();
    scene.addComponent<BoundingVolumeComponent>(ent, BoundingVolumeComponent(std::make_unique<AABBBV>(boundingBox)));


    for (int x = 0; x < 100; ++x) {
        for (int z = 0; z < 10; ++z) {
            EntityID ent = scene.createEntity();
            scene.getComponent<ObjectInfoComponent>(ent).name = "Nanosuit_" + std::to_string(x) + "_" + std::to_string(z);

            ts.translateEntity(ent, glm::vec3(x * 2.0f, 0.0f, z * 2.0f));
            ts.scaleEntity(ent, glm::vec3(0.1f, 0.1f, 0.1f));

            scene.addComponent<ModelComponent>(ent, { shaders[0], &ourModel });


            scene.addComponent<BoundingVolumeComponent>(ent, BoundingVolumeComponent(std::make_unique<AABBBV>(boundingBox)));
        }
    }

     ent = player = scene.createEntity();
     scene.getComponent<ObjectInfoComponent>(ent).name = "Wardrobe";
     //ts.scaleEntity(ent, glm::vec3(5.0f, 5.0f, 5.0f));

     scene.addComponent<ModelComponent>(ent, { shaders[1], &model4 });

     scene.addComponent(ent, BoundingVolumeComponent(std::make_unique<AABBBV>(model4.calculateBoundingBox())));
     scene.getTransformSystem().scaleEntity(ent,glm::vec3(0.001f));

	ent = scene.createEntity();
    scene.getComponent<ObjectInfoComponent>(ent).name = "Floor";

	ts.scaleEntity(ent, glm::vec3(5.0, 0.1f, 5.0f));

	scene.addComponent<ModelComponent>(ent, { shaders[0], &model3 });
    boundingBox = model3.calculateBoundingBox();
    scene.addComponent<BoundingVolumeComponent>(ent, BoundingVolumeComponent(std::make_unique<AABBBV>(boundingBox)));

	colliderComponent = &scene.addComponent<ColliderComponent>(ent, ColliderComponent(ColliderType::BOX, true));


    std::pair<glm::vec3, glm::vec3> wallScalesAndTranslations[] = {
        { glm::vec3(5.0f, 1.0f, 1.0f), glm::vec3(0.0f, 1.0f, 6.0f) },
        { glm::vec3(5.0f, 1.0f, 1.0f), glm::vec3(0.0f, 1.0f, -6.0f) },
        { glm::vec3(1.0f, 1.0f, 5.0f), glm::vec3(6.0f, 1.0f, 0.0f) },
        { glm::vec3(1.0f, 1.0f, 5.0f), glm::vec3(-6.0f, 1.0f, 0.0f) },
    };

	for (int i = 0; i < 4; ++i) {
		ent = scene.createEntity();
		scene.getComponent<ObjectInfoComponent>(ent).name = "Wall " + std::to_string(i + 1);

		ts.scaleEntity(ent, wallScalesAndTranslations[i].first);
		ts.translateEntity(ent, wallScalesAndTranslations[i].second);

		scene.addComponent<ModelComponent>(ent, { shaders[0], &model3 });
	    scene.addComponent<BoundingVolumeComponent>(ent, BoundingVolumeComponent(std::make_unique<AABBBV>(boundingBox)));
		colliderComponent = &scene.addComponent<ColliderComponent>(ent, ColliderComponent(ColliderType::BOX, true));
	}


	ent = scene.createEntity();
	scene.getComponent<ObjectInfoComponent>(ent).name = "Wall 5";

	ts.rotateEntity(ent, glm::vec3(0.0f, 30.0f, 180.0f));
	ts.translateEntity(ent, glm::vec3(1.0f, 1.0f, 2.0f));
	ts.scaleEntity(ent, glm::vec3(0.5f, 2.0f, 0.5f));

	scene.addComponent<ModelComponent>(ent, { shaders[0], &model3 });
    scene.addComponent<BoundingVolumeComponent>(ent, BoundingVolumeComponent(std::make_unique<AABBBV>(boundingBox)));
	colliderComponent = &scene.addComponent<ColliderComponent>(ent, ColliderComponent(ColliderType::BOX, true));


    ent = scene.createEntity();
    scene.getComponent<ObjectInfoComponent>(ent).name = "Cloud";

	scene.addComponent(ent, ImageComponent{ shaders[3], "res/textures/cloud.png" });
    ts.translateEntity(ent, glm::vec3(9*WINDOW_WIDTH/10, WINDOW_HEIGHT/10, 0.0f));
    ts.scaleEntity(ent, glm::vec3(250.0f));

    ent = scene.createEntity();
	scene.getComponent<ObjectInfoComponent>(ent).name = "Text";

	scene.addComponent(ent, TextComponent{ shaders[4], "foo", glm::vec4(1, 0, 0, 1), "text" });
    ts.translateEntity(ent, glm::vec3(1*WINDOW_WIDTH/10, WINDOW_HEIGHT/10, 0.0f));

    EventSystem& eventSystem = scene.getEventSystem();
    eventSystem.registerListener<CollisionEvent>([&](const Event& e) {
        const auto& event = static_cast<const CollisionEvent&>(e);
        if (event.isColliding)
        {
            spdlog::info("Collision detected between {} and {}", event.objectA, event.objectB);
        }
    });
////    scene.addChild(obj1);
////    obj1->addChild(obj2);
////    obj1->components.AddComponent<Transform>();
////    obj1->components.AddComponent<ModelComponent>(&ourModel);
////    obj2->components.AddComponent<Transform>();
////    obj2->components.AddComponent<ModelComponent>(&ourModel);
////
////    h1 = new HudElement(0.01 * WINDOW_WIDTH, 0.01 * WINDOW_HEIGHT, 0.3 * WINDOW_WIDTH, 0.05 * WINDOW_HEIGHT);
////    h1->setColor(glm::vec4(1,0,0,1));
////
////    h2 = new HudElement(0.8 * WINDOW_WIDTH, 0.0 * WINDOW_HEIGHT, 0.2 * WINDOW_WIDTH, 0.2 * WINDOW_HEIGHT);
////    h2->setTexture("../../res/textures/cloud.png");
////    hud.setRoot(h1);
////    h1->addChild(h2);

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


void game_input()
{
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS || player == (EntityID)-1)
		return;

    auto& transform = scene.getComponent<Transform>(player);
    glm::vec3 translation = transform.translation;
    glm::vec3 rotation = transform.eulerRotation;

    constexpr float moveSpeed = 3.0f;
    constexpr float rotateSpeed = 180.0f;
    bool change = false;

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        rotation.y += rotateSpeed * deltaTime;
        change = true;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        rotation.y -= rotateSpeed * deltaTime;
        change = true;
    }

    glm::quat quatRotation = glm::quat(glm::radians(rotation));
    glm::vec3 forward = quatRotation * glm::vec3(0.0f, 0.0f, 1.0f) * (moveSpeed * deltaTime);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        translation += forward;
        change = true;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        translation -= forward;
        change = true;
    }


    if (change)
    {
        auto& ts = scene.getTransformSystem();
        ts.translateEntity(player, translation);
        ts.rotateEntity(player, rotation);
    }
}

void input()
{
    static bool firstMouse = true;

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) != GLFW_PRESS)
    {
        firstMouse = true;
        return;
    }

    static float cameraSpeed = 1.0f;
    if (scrollYOffset != 0.0f)
    {
        cameraSpeed += scrollYOffset * 0.1f;
        cameraSpeed = glm::clamp(cameraSpeed, 0.1f, 2.0f);
    }
    float scaledCamSpeed = cameraSpeed * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.processKeyboard(FORWARD, scaledCamSpeed);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.processKeyboard(BACKWARD, scaledCamSpeed);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.processKeyboard(LEFT, scaledCamSpeed);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.processKeyboard(RIGHT, scaledCamSpeed);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera.processKeyboard(DOWN, scaledCamSpeed);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera.processKeyboard(UP, scaledCamSpeed);

    // Add mouse controls if needed
    static double lastX = WINDOW_WIDTH / 2.0f;
    static double lastY = WINDOW_HEIGHT / 2.0f;

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

    camera.processMouseMovement(xoffset, yoffset);
}

void update()
{
    auto& ts = scene.getTransformSystem();
    ts.update();

	auto& cs = scene.getCollisionSystem();
	cs.CheckCollisions();

    auto& collisions = cs.GetCollisions();
	spdlog::info("Collisions: {}", collisions.size());

	bool updateScene = false;

    for (const CollisionEvent& collision : collisions)
    {
		auto& transformA = scene.getComponent<Transform>(collision.objectA);
		auto& transformB = scene.getComponent<Transform>(collision.objectB);

        auto& colliderA = scene.getComponent<ColliderComponent>(collision.objectA);
        auto& colliderB = scene.getComponent<ColliderComponent>(collision.objectB);

		if (colliderA.isStatic && colliderB.isStatic)
        {
            continue;
        }

        updateScene = true;
        glm::vec3 separationVector = collision.separationVector;
        if (!colliderA.isStatic && !colliderB.isStatic)
        {
            separationVector /= 2.0f;
        }

        if (!colliderA.isStatic)
        {
			glm::mat4 newMatrix = transformA.globalMatrix;
			newMatrix[3] += glm::vec4(separationVector, 0.0f);
			ts.setGlobalMatrix(collision.objectA, newMatrix);
        }

        if (!colliderB.isStatic)
        {
			glm::mat4 newMatrix = transformB.globalMatrix;
			newMatrix[3] -= glm::vec4(separationVector, 0.0f);
			ts.setGlobalMatrix(collision.objectB, newMatrix);
        }
    }
    if (updateScene)
		ts.update();

    EventSystem& eventSystem = scene.getEventSystem();
    eventSystem.processEvents();
}

void render(const Framebuffer& framebuffer)
{
	framebuffer.Bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (!show_wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

	scene.getRenderingSystem().drawScene(framebuffer, camera);
	scene.getRenderingSystem().drawHud(framebuffer);
}

void imgui_begin()
{
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}


const char* HIERARCHY_NODE = "HIERARCHY_NODE";

void imgui_hierarchy(Scene* scene);
void imgui_hierarchy_node(Scene* scene, EntityID id);
void imgui_rearrange_target(Scene* scene, EntityID id, int targetIndex);

void imgui_inspector(Scene* scene);
void imgui_transform(Scene* scene, EntityID id);
void imgui_collider(Scene* scene, EntityID id);
void imgui_model(Scene* scene, EntityID id);

void imgui_shaders();
void imgui_uniform_descendants(const Shader& shader, const std::vector<UniformInfo>& uniforms, const std::string& fullName);
void imgui_uniform_struct(const Shader& shader, const UniformInfo& uniform, const std::string& fullName, int index);
void imgui_uniform_array(const Shader& shader, const UniformInfo& uniform, const std::string& fullName);
void imgui_uniform_leaf(const Shader& shader, const UniformInfo& uniform, const std::string& fullName, int index);

void imgui_scene();



void imgui_render()
{
    static bool show_demo_window = false;
    static bool show_collisions = false;

    ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("Settings"))
        {
            ImGui::MenuItem("Demo Window", NULL, &show_demo_window);
            ImGui::MenuItem("Collisions", NULL, &show_collisions);
            ImGui::MenuItem("Wireframe", NULL, &show_wireframe);

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

        std::string fpsText = fmt::format("{:.1f} FPS", ImGui::GetIO().Framerate);
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


    imgui_scene();

    imgui_hierarchy(&scene);
	imgui_inspector(&scene);
    imgui_shaders();

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

void imgui_hierarchy(Scene* scene)
{
    constexpr float rightMargin = 40.0f;

    ImGui::SetNextWindowSize(ImVec2(500, 550), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Hierarchy"))
    {
        if (ImGui::BeginChild("HierarchyScrollArea", ImVec2(0, 0),
            ImGuiChildFlags_None,
            ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoSavedSettings
        ))
        {
			EntityID root = scene->getSceneRootEntity();

            if (ImGui::BeginPopupContextWindow("Hierarchy Context Menu"))
            {
                bool disabled;
                if (ImGui::MenuItem("Add Object"))
                {
					EntityID newObject = scene->createEntity(root);
					scene->getComponent<ObjectInfoComponent>(newObject).name = "New Object";
                }

                ImGui::EndPopup();
            }

			imgui_hierarchy_node(scene, root);
        }
        ImGui::EndChild();
    }
    ImGui::End();
}

void imgui_hierarchy_node(Scene* scene, EntityID id)
{
    ImGui::PushID(id);

	std::string objName = scene->getComponent<ObjectInfoComponent>(id).name;
    std::string displayName = objName + "###objName";
	Transform& transform = scene->getComponent<Transform>(id);


    ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick
        | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding;

    nodeFlags |= (transform.children.size() == 0) ? ImGuiTreeNodeFlags_Leaf : ImGuiTreeNodeFlags_DefaultOpen;
    if (id == selectedObject)
        nodeFlags |= ImGuiTreeNodeFlags_Selected;


    // tree node start
    bool opened = ImGui::TreeNodeEx(displayName.c_str(), nodeFlags);

    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
    {
        selectedObject = id;
    }

    // Context Menu
    bool deleteObject = false;
    if (ImGui::BeginPopupContextItem("Context Menu"))
    {
        bool disabled;

        if (ImGui::MenuItem("Add Child"))
        {
			EntityID newObject = scene->createEntity(id);
			scene->getComponent<ObjectInfoComponent>(newObject).name = "New Object";
        }

        if (disabled = id == scene->getSceneRootEntity()) ImGui::BeginDisabled();
        if (ImGui::MenuItem("Delete"))
        {
            deleteObject = true;
        }
        if (disabled) ImGui::EndDisabled();

        ImGui::EndPopup();
    }


    // Drag and Drop
    if (ImGui::BeginDragDropSource())
    {
		ImGui::SetDragDropPayload(HIERARCHY_NODE, &id, sizeof(EntityID));
        ImGui::Text(objName.c_str());
        ImGui::EndDragDropSource();
    }
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(HIERARCHY_NODE))
        {
			EntityID draggedObj = *(EntityID*)payload->Data;
			scene->getTransformSystem().addChildKeepTransform(id, draggedObj);
        }
        ImGui::EndDragDropTarget();
    }

    if (opened)
    {
        if (transform.children.size() > 0)
        {
            for (int i = 0; i < transform.children.size(); i++)
            {
				imgui_rearrange_target(scene, id, i);
				imgui_hierarchy_node(scene, transform.children[i]);
            }
			imgui_rearrange_target(scene, id, transform.children.size());
        }

        ImGui::TreePop();
    }
    ImGui::PopID();

    if (deleteObject)
    {
		scene->destroyEntity(id);
    }
}

void imgui_rearrange_target(Scene* scene, EntityID id, int targetIndex)
{
    const ImGuiPayload* payload = ImGui::GetDragDropPayload();

    if (payload && payload->IsDataType(HIERARCHY_NODE))
    {
        float separatorThickness = ImGui::GetStyle().ItemSpacing.y;
        float cursorPosY = ImGui::GetCursorPosY();

        ImGui::SetCursorPosY(cursorPosY - separatorThickness);
        ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal, separatorThickness);

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(HIERARCHY_NODE))
            {
				EntityID draggedObj = *(EntityID*)payload->Data;
				auto& ts = scene->getTransformSystem();
				ts.addChildKeepTransform(id, draggedObj);
				ts.setChildIndex(id, draggedObj, targetIndex);
                
            }
            ImGui::EndDragDropTarget();
        }
        ImGui::SetCursorPosY(cursorPosY);
    }
}

void imgui_inspector(Scene* scene)
{
    ImGui::SetNextWindowSize(ImVec2(500, 550), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Inspector"))
    {
        if (selectedObject != (EntityID)-1)
        {
			assert(scene->hasComponent<ObjectInfoComponent>(selectedObject));
			auto& objectInfo = scene->getComponent<ObjectInfoComponent>(selectedObject);
			std::string objName = objectInfo.name;
            std::string displayName = objName + "###objName";
            const char* name = objName.c_str();

            if (ImGui::InputText("Name", (char*)name, 64))
            {
				objectInfo.name = name;
            }

            imgui_transform(scene, selectedObject);
            imgui_collider(scene, selectedObject);
            imgui_model(scene, selectedObject);
        }
    }
    ImGui::End();
}


void imgui_transform(Scene* scene, EntityID id)
{
	if (!scene->hasComponent<Transform>(id))
		return;

	auto& ts = scene->getTransformSystem();
	auto& transform = scene->getComponent<Transform>(id);

    ImGui::PushID(&transform);

    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
    {
		glm::vec3 translation = transform.translation;
		glm::vec3 rotation = transform.eulerRotation;
		glm::vec3 scale = transform.scale;

        if (ImGui::DragFloat3("Position", &translation[0], 0.1f))
        {
			ts.translateEntity(id, translation);
        }
        if (ImGui::DragFloat3("Rotation", &rotation[0], 0.1f))
        {
			ts.rotateEntity(id, rotation);
        }
        if (ImGui::DragFloat3("Scale", &scale[0], 0.1f))
        {
			ts.scaleEntity(id, scale);
        }
    }

    ImGui::PopID();
}

void imgui_collider(Scene* scene, EntityID id)
{
	if (!scene->hasComponent<ColliderComponent>(id))
		return;
	auto& collider = scene->getComponent<ColliderComponent>(id);

    ImGui::PushID(&collider);

    ColliderShape* shape = collider.GetColliderShape();
    std::string tabName;

    switch (shape->getType())
    {
    case ColliderType::BOX:
        tabName = "Box Collider";
        break;
    case ColliderType::SPHERE:
        tabName = "Sphere Collider";
        break;
    default:
        tabName = "Unknown Collider";
        break;
    }

    if (ImGui::CollapsingHeader(tabName.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
    {
        glm::vec3 center = shape->center;
        if (ImGui::DragFloat3("Center", &center[0], 0.1f))
        {
            shape->center = center;
        }

        switch (shape->getType())
        {
        case ColliderType::BOX:
            ImGui::DragFloat3("Half Size", &((BoxCollider*)shape)->halfSize[0], 0.1f, 0.0f, 1000.0f);
            break;
        case ColliderType::SPHERE:
            ImGui::DragFloat("Radius", &((SphereCollider*)shape)->radius, 0.1f, 0.0f, 1000.0f);
            break;
        default:
            break;
        }
    }


    ImGui::PopID();
}

void imgui_model(Scene* scene, EntityID id)
{
	if (!scene->hasComponent<ModelComponent>(id))
		return;
	auto& modelComponent = scene->getComponent<ModelComponent>(id);
	ImGui::PushID(&modelComponent);

    if (ImGui::CollapsingHeader("Model", ImGuiTreeNodeFlags_DefaultOpen))
    {
		Shader* modelShader = modelComponent.shader;
        if (ImGui::BeginCombo("Shader##Combo", modelShader->getName().c_str()))
        {
            for (int i = 0; i < shaders.size(); i++)
            {
                if (ImGui::Selectable(shaders[i]->getName().c_str(), modelShader == shaders[i]))
                {
                    modelShader = shaders[i];
					modelComponent.shader = modelShader;
                }
            }

            ImGui::EndCombo();
        }
    }


    ImGui::PopID();
}



void imgui_shaders()
{
    ImGui::SetNextWindowSize(ImVec2(500, 550), ImGuiCond_FirstUseEver);
    ImGui::Begin("Shaders");
    static Shader* shader = shaders[0];
    if (ImGui::BeginCombo("Shader##Combo", shader->getName().c_str()))
    {
        for (int i = 0; i < shaders.size(); i++)
        {
            if (ImGui::Selectable(shaders[i]->getName().c_str(), shader == shaders[i]))
            {
                shader = shaders[i];
            }
        }
        ImGui::EndCombo();
    }
    ImGui::Dummy(ImVec2(0, 15));
    shader->use();

    imgui_uniform_descendants(*shader, shader->getUniforms(), "");

    ImGui::End();
}

void imgui_uniform_descendants(const Shader& shader, const std::vector<UniformInfo>& uniforms, const std::string& fullName)
{
    for (auto& uniform : uniforms)
    {
        if (uniform.size > 1)
            imgui_uniform_array(shader, uniform, fullName + uniform.name);
        else if (uniform.members.size() > 0)
            imgui_uniform_struct(shader, uniform, fullName + uniform.name, -1);
        else
            imgui_uniform_leaf(shader, uniform, fullName + uniform.name, -1);

    }
    ImGui::Dummy(ImVec2(0, 10));
}

void imgui_uniform_array(const Shader& shader, const UniformInfo& uniform, const std::string& fullName)
{
    std::string displayString = uniform.name + "[]";
    ImGui::PushID(fullName.c_str());
    if (ImGui::TreeNodeEx(displayString.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth
        | ImGuiTreeNodeFlags_FramePadding))
    {
        for (int i = 0; i < uniform.size; i++)
        {
            std::string uniformName = fullName + "[" + std::to_string(i) + "]";

            if (uniform.members.empty())
                imgui_uniform_leaf(shader, uniform, uniformName, i);
            else
                imgui_uniform_struct(shader, uniform, uniformName, i);

            ImGui::Spacing();
        }

        ImGui::TreePop();
    }
    ImGui::PopID();

}

void imgui_uniform_struct(const Shader& shader, const UniformInfo& uniform, const std::string& fullName, int index)
{
    std::string displayString = uniform.name;
    if (index >= 0)
    {
        displayString += "[" + std::to_string(index) + "]";
    }
    ImGui::PushID(fullName.c_str());

    if (ImGui::TreeNodeEx(displayString.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth
        | ImGuiTreeNodeFlags_FramePadding))
    {
        imgui_uniform_descendants(shader, uniform.members, fullName + '.');
        ImGui::TreePop();
    }
    ImGui::PopID();

}

void imgui_uniform_leaf(const Shader& shader, const UniformInfo& uniform, const std::string& fullName, int index)
{
    if (uniform.name == "projection" || uniform.name == "view" || uniform.name == "model")
        return;

    ImGui::PushID(fullName.c_str());

    GLuint location = glGetUniformLocation(shader.ID, fullName.c_str());
    if (location == -1)
        ImGui::BeginDisabled();

    std::string label;
    bool treeOpen = false;
    if (index >= 0)
    {
        label = std::to_string(index);
    }
    else
    {
        label = uniform.name;
    }

    bool changed = false;
    switch (uniform.type)
    {
    case GL_FLOAT:
        GLfloat floatValue;
        glGetUniformfv(shader.ID, location, &floatValue);
        if (ImGui::DragFloat(label.c_str(), &floatValue, 0.01f))
        {
            shader.setFloat(fullName, floatValue);
        }
        break;
    case GL_INT:
        GLint intValue;
        glGetUniformiv(shader.ID, location, &intValue);
        if (ImGui::InputInt(label.c_str(), &intValue))
        {
            shader.setInt(fullName, intValue);
        }
        break;
    case GL_BOOL:
        GLint boolValue;
        glGetUniformiv(shader.ID, location, &boolValue);
        if (ImGui::Checkbox(label.c_str(), (bool*)&boolValue))
        {
            shader.setBool(fullName, boolValue);
        }
        break;
    case GL_FLOAT_VEC2:
        glm::vec2 vec2Value;
        glGetUniformfv(shader.ID, location, &vec2Value[0]);
        if (ImGui::DragFloat2(label.c_str(), &vec2Value[0], 0.01f))
        {
            shader.setVec2(fullName, vec2Value);
        }
        break;
    case GL_FLOAT_VEC3:
        glm::vec3 vec3Value;
        glGetUniformfv(shader.ID, location, &vec3Value[0]);
        if (ImGui::DragFloat3(label.c_str(), &vec3Value[0], 0.01f))
        {
            shader.setVec3(fullName, vec3Value);
        }
        break;
    case GL_FLOAT_VEC4:
        glm::vec4 vec4Value;
        glGetUniformfv(shader.ID, location, &vec4Value[0]);
        if (ImGui::DragFloat4(label.c_str(), &vec4Value[0], 0.01f))
        {
            shader.setVec4(fullName, vec4Value);
        }
        break;
    case GL_FLOAT_MAT2:
        glm::mat2 mat2Value;
        glGetUniformfv(shader.ID, location, &mat2Value[0][0]);

        changed = false;
        for (int i = 0; i < 2; ++i)
        {
            changed |= ImGui::DragFloat2((label + "##" + std::to_string(i)).c_str(), &mat2Value[i][0], 0.01f);
            label.clear();
        }
        if (changed)
        {
            shader.setMat2(fullName, mat2Value);
        }
        break;
    case GL_FLOAT_MAT3:
        glm::mat3 mat3Value;
        glGetUniformfv(shader.ID, location, &mat3Value[0][0]);

        changed = false;
        for (int i = 0; i < 3; ++i)
        {
            changed |= ImGui::DragFloat3((label + "##" + std::to_string(i)).c_str(), &mat3Value[i][0], 0.01f);
            label.clear();
        }
        if (changed)
        {
            shader.setMat3(fullName, mat3Value);
        }
        break;
    case GL_FLOAT_MAT4:
        glm::mat4 mat4Value;
        glGetUniformfv(shader.ID, location, &mat4Value[0][0]);

        changed = false;
        for (int i = 0; i < 4; ++i)
        {
            changed |= ImGui::DragFloat4((label + "##" + std::to_string(i)).c_str(), &mat4Value[i][0], 0.01f);
            label.clear();
        }
        if (changed)
        {
            shader.setMat4(fullName, mat4Value);
        }
        break;
    default:
        ImGui::TreeNodeEx(uniform.name.c_str(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth
            | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen);
        break;
    }

    if (location == -1)
        ImGui::EndDisabled();
    ImGui::PopID();

}




void imgui_scene()
{
    static ImVec2 lastSize = ImVec2(0, 0);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(650, 400), ImGuiCond_FirstUseEver);
    ImGui::Begin("Scene");

    //ImVec2 mousePos = ImGui::GetMousePos();
    //ImVec2 imguiCursorPos = ImGui::GetCursorScreenPos();
    //ImVec2 relativeCursorPos = ImVec2(mousePos.x - imguiCursorPos.x, mousePos.y - imguiCursorPos.y);

    if (!ImGui::IsAnyItemActive())
    {
        if (ImGui::IsWindowFocused())
        {
            game_input();
        }
        if (ImGui::IsWindowHovered())
        {
            input();
        }
    }

    ImVec2 size = ImGui::GetContentRegionAvail();
    if (size.x != lastSize.x || size.y != lastSize.y)
    {
        sceneFramebuffer->Resize(size.x, size.y);
        lastSize = size;
    }

    GLuint texture = sceneFramebuffer->GetColorTexture();
    ImGui::Image(texture, size, ImVec2(0, 1), ImVec2(1, 0));

    ImGui::End();
    ImGui::PopStyleVar();
}



void imgui_end()
{
    int display_w, display_h;
    glfwMakeContextCurrent(window);
    glfwGetFramebufferSize(window, &display_w, &display_h);

	DefaultFramebuffer::GetInstance().Bind();
    glViewport(0, 0, display_w, display_h);
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void end_frame()
{
	scrollXOffset = scrollYOffset = 0.0f;
    glfwPollEvents();
    glfwMakeContextCurrent(window);
    glfwSwapBuffers(window);
}

