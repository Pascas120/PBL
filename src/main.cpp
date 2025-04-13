// dear imgui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// If you are new to dear imgui, see examples/README.txt and documentation at the top of imgui.cpp.
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan graphics context creation, etc.)

#include <memory>

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
void imgui_viewport();

void end_frame();

constexpr int32_t WINDOW_WIDTH  = 1920;
constexpr int32_t WINDOW_HEIGHT = 1080;

GLFWwindow* window = nullptr;

// Change these to lower GL version like 4.5 if GL 4.6 can't be initialized on your machine
const     char*   glsl_version     = "#version 410";
constexpr int32_t GL_VERSION_MAJOR = 4;
constexpr int32_t GL_VERSION_MINOR = 1;

bool show_wireframe = false;
bool show_colliders = true;

ImVec4 clear_color         = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
float deltaTime = 0.0f;
float lastFrame = 0.0f;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
Shader ourShader;
Model ourModel;

Scene scene;
GameObject* obj1 = new GameObject();
GameObject* obj2 = new GameObject();
GameObject* obj3 = new GameObject();
GameObject* obj4 = new GameObject();
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
        input();

        // Update game objects' state here
        update();

        // OpenGL rendering code here
		render(*sceneFramebuffer); // mo¿na tak robiæ z unique_ptrami?

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
    stbi_set_flip_vertically_on_load(true);
    ourShader = Shader("../../res/shaders/basic.vert", "../../res/shaders/basic.frag");
    ourModel = Model("../../res/models/nanosuit/nanosuit.obj");

	scene.GetRoot()->SetName("Root");

    scene.addChild(obj1);
    obj1->AddChild(obj2);
    obj1->components.AddComponent<Transform>();
    obj1->components.AddComponent<ModelComponent>(&ourModel);
    //obj1->components.AddComponent<ColliderComponent>(ColliderType::BOX);
    obj1->SetName("Parent");

    obj2->components.AddComponent<Transform>();
    obj2->components.AddComponent<ModelComponent>(&ourModel);
	obj2->components.AddComponent<ColliderComponent>(ColliderType::BOX);
	obj2->SetName("Child A");

	obj1->AddChild(obj3);
	obj3->components.AddComponent<Transform>();
	obj3->components.AddComponent<ColliderComponent>(ColliderType::SPHERE);
	obj3->SetName("Child B");

	obj1->AddChild(obj4);
	obj4->components.AddComponent<Transform>();
	obj4->components.AddComponent<ColliderComponent>(ColliderType::BOX);
	obj4->SetName("Child C");

//==============================================================================================

	glfwSetScrollCallback(window, [](GLFWwindow* window, double xoffset, double yoffset) {
		camera.ProcessMouseScroll(yoffset);
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

void input()
{
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
}

void update()
{
    scene.Update();
	scene.GetCollisionSystem()->CheckCollisions();

    std::vector<CollisionInfo> collisions = scene.GetCollisionSystem()->GetCollisions();
    for (const CollisionInfo& collision : collisions)
    {
		Transform* transformA = collision.objectA->components.GetComponent<Transform>();
		Transform* transformB = collision.objectB->components.GetComponent<Transform>();

		transformA->setTranslation(transformA->getTranslation() + collision.separationVector / 2.0f);
		collision.objectA->MarkDirty();
		transformB->setTranslation(transformB->getTranslation() - collision.separationVector / 2.0f);
		collision.objectB->MarkDirty();
    }
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
		GameObject* obj[] = { obj1, obj2, obj3, obj4 };
        for (int i = 0; i < 4; i++)
        {
            Transform* transform = obj[i]->components.GetComponent<Transform>();
            ColliderComponent* collider = obj[i]->components.GetComponent<ColliderComponent>();
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
    DefaultFramebuffer::GetInstance().Bind();
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
			ImGui::EndMenu();
		}
		
        ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x);
		ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
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

	imgui_viewport();


    {
        ImGui::Begin("Hello, world!");

        //ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

		ImGui::NewLine();

        imgui_obj_info(scene.GetRoot());


        ImGui::End();
    }
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

bool resize = false;
ImVec2 newSize = ImVec2(0, 0);

void imgui_viewport()
{
    static ImVec2 lastSize = ImVec2(0, 0);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::Begin("Viewport");

	ImVec2 size = ImGui::GetContentRegionAvail();
	if (size.x != lastSize.x || size.y != lastSize.y)
	{
		//sceneFramebuffer->Resize(size.x, size.y);
		lastSize = size;
		resize = true;
		newSize = size;
	}

	GLuint texture = sceneFramebuffer->GetColorTexture();
	ImGui::Image(texture, size, ImVec2(0, 1), ImVec2(1, 0));

	ImGui::End();
	ImGui::PopStyleVar();
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

    if (resize)
    {
        sceneFramebuffer->Resize(newSize.x, newSize.y);
        resize = false;
    }
}

void end_frame()
{
    glfwPollEvents();
    glfwMakeContextCurrent(window);
    glfwSwapBuffers(window);
}

