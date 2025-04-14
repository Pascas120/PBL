// dear imgui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// If you are new to dear imgui, see examples/README.txt and documentation at the top of imgui.cpp.
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan graphics context creation, etc.)

#include <memory>

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl/imgui_impl_glfw.h"
#include "imgui_impl/imgui_impl_opengl3.h"

#include "glm/gtc/quaternion.hpp"

//#include <stdio.h>
#include "Shader.h"
#include "stb_image.h"
#include "Model.h"
#include "Camera.h"
#include "Scene.h"
#include "Transform.h"
#include "ModelComponent.h"
#include "ColliderComponent.h"

#define IMGUI_IMPL_OPENGL_LOADER_GLAD

#include <glad/glad.h>  // Initialize with gladLoadGL()
#include <GLFW/glfw3.h> // Include glfw3.h after our OpenGL definitions
#include <spdlog/spdlog.h>

#include "debug.h"
#include "Framebuffer.h"

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

void imgui_obj_info(GameObject* obj);
void imgui_transform(GameObject* obj);
void imgui_collider(GameObject* obj);
void imgui_children(GameObject* obj);
void imgui_collisions(bool& show);
void imgui_scene();

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
bool show_colliders = true;

ImVec4 clear_color         = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
float deltaTime = 0.0f;
float lastFrame = 0.0f;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
Shader ourShader;
Model ourModel;
Model model2;
Model model3;

Scene scene;
//GameObject* obj1 = new GameObject();
//GameObject* obj2 = new GameObject();
//GameObject* obj3 = new GameObject();
//GameObject* obj4 = new GameObject();

std::vector<GameObject*> objects;
GameObject* player = nullptr;
float osc = 0;

//GLuint fbo;
//GLuint texture;
//GLuint depthTexture;

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

        // Process I/O operations here
        // input();

        // Draw ImGui
        imgui_begin();
        imgui_render(); // edit this function to add your own ImGui controls

        // Update game objects' state here
        update();
        // OpenGL rendering code here
		render(*sceneFramebuffer); // można tak robić z unique_ptrami?

        
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

	for (GameObject* obj : objects)
	{
		if (obj)
		{
			delete obj;
			obj = nullptr;
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

	sceneFramebuffer = std::make_unique<CustomFramebuffer>(FramebufferConfig{ WINDOW_WIDTH, WINDOW_HEIGHT });


//==============================================================================================
    glEnable(GL_DEPTH_TEST);
    ourShader = Shader("res/shaders/basic.vert", "res/shaders/basic.frag");
    ourModel = Model("res/models/nanosuit/nanosuit.obj");
	model2 = Model("res/models/dee/waddledee.obj");
	model3 = Model("res/models/grass_block/grass_block.obj");

    objects.reserve(10);
	scene.GetRoot()->SetName("Root");

	GameObject* obj = new GameObject();
	objects.emplace_back(obj);
    scene.addChild(obj);
    
    obj->components.AddComponent<Transform>();
    obj->components.GetComponent<Transform>()->setScale(glm::vec3(5.0f, 5.0f, 5.0f));
    obj->components.AddComponent<ModelComponent>(&model2);
    obj->components.AddComponent<ColliderComponent>(ColliderType::SPHERE);

    SphereCollider* sphereCollider = static_cast<SphereCollider*>(obj->components.GetComponent<ColliderComponent>()->GetColliderShape());
    sphereCollider->center = glm::vec3(-0.01f, 0.1f, 0.01f);
    sphereCollider->radius = 0.1f;
    obj->SetName("Player");
	player = obj;


	obj = new GameObject();
	objects.emplace_back(obj);
	scene.addChild(obj);

	obj->components.AddComponent<Transform>();
	Transform* transform = obj->components.GetComponent<Transform>();
	transform->setScale(glm::vec3(0.1f, 0.1f, 0.1f));
	transform->setTranslation(glm::vec3(2.5f, 0.0f, 0.0f));
	obj->components.AddComponent<ModelComponent>(&ourModel);
	obj->components.AddComponent<ColliderComponent>(ColliderType::BOX);

	BoxCollider* boxCollider = static_cast<BoxCollider*>(obj->components.GetComponent<ColliderComponent>()->GetColliderShape());
	boxCollider->center = glm::vec3(0.0f, 7.7f, 0.0f);
	boxCollider->halfSize = glm::vec3(4.0f, 7.7f, 1.778f);
	obj->SetName("Nanosuit");


    obj = new GameObject();
    objects.emplace_back(obj);
    scene.addChild(obj);

    obj->components.AddComponent<Transform>();
	transform = obj->components.GetComponent<Transform>();
	transform->setScale(glm::vec3(5.0f, 0.1f, 5.0f));

	obj->components.AddComponent<ModelComponent>(&model3);
	obj->components.AddComponent<ColliderComponent>(ColliderType::BOX, true);
	obj->SetName("Floor");

	std::pair<glm::vec3, glm::vec3> wallScalesAndTranslations[] = {
		{ glm::vec3(5.0f, 1.0f, 1.0f), glm::vec3(0.0f, 1.0f, 6.0f) },
		{ glm::vec3(5.0f, 1.0f, 1.0f), glm::vec3(0.0f, 1.0f, -6.0f) },
		{ glm::vec3(1.0f, 1.0f, 5.0f), glm::vec3(6.0f, 1.0f, 0.0f) },
		{ glm::vec3(1.0f, 1.0f, 5.0f), glm::vec3(-6.0f, 1.0f, 0.0f) },
	};

	for (int i = 0; i < 4; ++i)
	{
		obj = new GameObject();
		objects.emplace_back(obj);
		scene.addChild(obj);

		obj->components.AddComponent<Transform>();
		transform = obj->components.GetComponent<Transform>();
		transform->setScale(wallScalesAndTranslations[i].first);
		transform->setTranslation(wallScalesAndTranslations[i].second);

		obj->components.AddComponent<ModelComponent>(&model3);
		obj->components.AddComponent<ColliderComponent>(ColliderType::BOX, true);
		obj->SetName("Wall " + std::to_string(i + 1));
	}

	obj = new GameObject();
	objects.emplace_back(obj);
	scene.addChild(obj);

	obj->components.AddComponent<Transform>();
	transform = obj->components.GetComponent<Transform>();
	transform->setRotation(glm::vec3(0.0f, 30.0f, 180.0f));
	transform->setTranslation(glm::vec3(1.0f, 1.0f, 2.0f));
	transform->setScale(glm::vec3(0.5f, 2.0f, 0.5f));

	obj->components.AddComponent<ModelComponent>(&model3);
	obj->components.AddComponent<ColliderComponent>(ColliderType::BOX, true);
	obj->SetName("Wall 5");

//==============================================================================================

	glfwSetScrollCallback(window, [](GLFWwindow* window, double xoffset, double yoffset) {
		//camera.ProcessMouseScroll(yoffset);
		scrollXOffset += xoffset;
		scrollYOffset += yoffset;
		});

	Debug::Init();


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
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
		return;

	spdlog::info("Game input");
	Transform* transform = player->components.GetComponent<Transform>();
	glm::vec3 translation = transform->getTranslation();
	glm::vec3 rotation = transform->getRotation();

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
		transform->setTranslation(translation);
		transform->setRotation(rotation);
		player->MarkDirty();
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
        camera.ProcessKeyboard(FORWARD, scaledCamSpeed);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, scaledCamSpeed);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, scaledCamSpeed);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, scaledCamSpeed);
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		camera.ProcessKeyboard(DOWN, scaledCamSpeed);
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		camera.ProcessKeyboard(UP, scaledCamSpeed);

    // Add mouse controls if needed
    static double lastX = WINDOW_WIDTH/2.0f;
    static double lastY = WINDOW_HEIGHT/2.0f;

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

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void update()
{
    scene.Update();
	scene.GetCollisionSystem()->CheckCollisions();

	bool updateScene = false;

    std::vector<CollisionInfo> collisions = scene.GetCollisionSystem()->GetCollisions();
    for (const CollisionInfo& collision : collisions)
    {
		Transform* transformA = collision.objectA->components.GetComponent<Transform>();
		Transform* transformB = collision.objectB->components.GetComponent<Transform>();

		ColliderComponent* colliderA = collision.objectA->components.GetComponent<ColliderComponent>();
		ColliderComponent* colliderB = collision.objectB->components.GetComponent<ColliderComponent>();

		if (colliderA->isStatic && colliderB->isStatic)
		{
			continue;
		}

		updateScene = true;
		glm::vec3 separationVector = collision.separationVector;
		if (!colliderA->isStatic && !colliderB->isStatic)
		{
			separationVector /= 2.0f;
		}

		if (!colliderA->isStatic)
        {
			transformA->setTranslation(transformA->getTranslation() + separationVector);
            collision.objectA->MarkDirty();
        }

		if (!colliderB->isStatic)
        {
			transformB->setTranslation(transformB->getTranslation() - separationVector);
            collision.objectB->MarkDirty();
        }
    }

	if (updateScene)
		scene.Update();
}

void render(const Framebuffer& framebuffer)
{
	//glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	//sceneFramebuffer->Bind();

	framebuffer.Bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (!show_wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }else{
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    ourShader.use();
	//FramebufferConfig config = sceneFramebuffer->GetConfig();
	uint32_t width, height;
	framebuffer.GetSize(width, height);

	//glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)config.width / (float)config.height, 0.1f, 100.0f);
	glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)width / (float)height, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    ourShader.setMat4("projection", projection);
    ourShader.setMat4("view", view);

    scene.Draw(ourShader);

	Debug::SetProjectionView(projection, view);

    if (show_colliders)
    {
        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
		for (GameObject* obj : objects)
        {
            Transform* transform = obj->components.GetComponent<Transform>();
            ColliderComponent* collider = obj->components.GetComponent<ColliderComponent>();
            if (collider)
            {
                Debug::DrawCollider(collider->GetColliderShape(), transform, glm::vec3(1.0f, 0.0f, 0.0f));
            }
        }
        glDepthMask(GL_TRUE);
        glEnable(GL_DEPTH_TEST);
    }
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
			ImGui::MenuItem("Show colliders", NULL, &show_colliders);
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

    if (show_collisions)
        imgui_collisions(show_collisions);

	imgui_scene();
	
    {
		ImGui::SetNextWindowSize(ImVec2(500, 550), ImGuiCond_FirstUseEver);
        ImGui::Begin("Hello, world!");

        //ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		//ImGui::NewLine();

        imgui_obj_info(scene.GetRoot());


        ImGui::End();
    }
    

    ImGui::Render();
}

void imgui_obj_info(GameObject* obj)
{
	ImGui::PushID(obj);

    std::string objName = obj->GetName();
    std::string displayName = objName + "###objName";
    const char* name = objName.c_str();


	bool header_open = ImGui::CollapsingHeader(displayName.c_str(), ImGuiTreeNodeFlags_DefaultOpen);
    if (ImGui::BeginPopupContextItem("Object Context Menu"))
    {
        if (ImGui::InputText("Name", (char*)name, 64))
        {
            obj->SetName(name);
        }

        ImGui::EndPopup();
    }
	if (header_open)
	{

		ImGui::Indent();
		imgui_transform(obj);
		imgui_collider(obj);
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

void imgui_collider(GameObject* obj)
{
	ColliderComponent* collider = obj->components.GetComponent<ColliderComponent>();

	if (!collider)
		return;

	ImGui::PushID(collider);

	ColliderShape* shape = collider->GetColliderShape();
    std::string tabName;

	switch (shape->getType())
	{
	case ColliderType::BOX:
		tabName = "Box Collider";
		break;
	case ColliderType::SPHERE:
		tabName = "Sphere Collider";
		break;
	case ColliderType::CAPSULE:
		tabName = "Capsule Collider";
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
			obj->MarkDirty();
		}

		switch (shape->getType())
		{
		case ColliderType::BOX:
			ImGui::DragFloat3("Half Size", &((BoxCollider*)shape)->halfSize[0], 0.1f, 0.0f, 1000.0f);
			break;
		case ColliderType::SPHERE:
			ImGui::DragFloat("Radius", &((SphereCollider*)shape)->radius, 0.1f, 0.0f, 1000.0f);
			break;
        case ColliderType::CAPSULE:
        {
            CapsuleCollider* capsule = (CapsuleCollider*)shape;
			ImGui::DragFloat("Radius", &capsule->radius, 0.1f, 0.0f, 1000.0f);
			ImGui::DragFloat("Height", &capsule->height, 0.1f, 0.0f, 1000.0f);
        }
			break;
		default:
			break;
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

void imgui_collisions(bool& show)
{
    ImGui::Begin("Collisions", &show);
	ImGui::Checkbox("Show Colliders", &show_colliders);

    std::vector<CollisionInfo> collisions = scene.GetCollisionSystem()->GetCollisions();
    for (const CollisionInfo& collision : collisions)
    {
		ImGui::Text("Object A: %s", collision.objectA->GetName().c_str());
		ImGui::Text("Object B: %s", collision.objectB->GetName().c_str());
        ImGui::Text("Separation Vector: (%f, %f, %f)", collision.separationVector.x, collision.separationVector.y, collision.separationVector.z);
        ImGui::Separator();
    }

    ImGui::End();
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

	if (!ImGui::IsAnyItemActive())// && ImGui::IsWindowHovered())
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

