// dear imgui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// If you are new to dear imgui, see examples/README.txt and documentation at the top of imgui.cpp.
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan graphics context creation, etc.)

#include <memory>
#include <algorithm>

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

#include "nlohmann/json.hpp"
#include <fstream>

using json = nlohmann::json;

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

void remove_object(GameObject* obj);

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
bool show_colliders = false;

ImVec4 clear_color         = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
float deltaTime = 0.0f;
float lastFrame = 0.0f;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

std::vector<Shader*> shaders;

//Model ourModel;
//Model model2;
//Model model3;

std::vector<Model*> models;

Scene scene;

std::vector<GameObject*> objects;
GameObject* player = nullptr;

GameObject* selectedObject = nullptr;

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

	for (Shader* shader : shaders)
	{
		if (shader)
		{
			delete shader;
			shader = nullptr;
		}
	}

	for (GameObject* obj : objects)
	{
		if (obj)
		{
			delete obj;
			obj = nullptr;
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

	sceneFramebuffer = std::make_unique<CustomFramebuffer>(FramebufferConfig{ WINDOW_WIDTH, WINDOW_HEIGHT });


//==============================================================================================
    glEnable(GL_DEPTH_TEST);

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
		new Shader("Dithered transparency - IW", {
				{ GL_VERTEX_SHADER, "res/shaders/basic.vert" },
				{ GL_FRAGMENT_SHADER, "res/shaders/dither.frag" }
			}));
	shaders.emplace_back(
		new Shader("Phong - DO", {
				{ GL_VERTEX_SHADER, "res/shaders/phong.vert" },
				{ GL_FRAGMENT_SHADER, "res/shaders/phong.frag" }
			}));
	shaders.emplace_back(
		new Shader("Glow - MH", {
				{ GL_VERTEX_SHADER, "res/shaders/glow.vert" },
				{ GL_FRAGMENT_SHADER, "res/shaders/glow.frag" }
			}));


    /*ourModel = Model("res/models/nanosuit/nanosuit.obj");
	model2 = Model("res/models/dee/waddledee.obj");
	model3 = Model("res/models/grass_block/grass_block.obj");*/

	models.emplace_back(new Model("res/models/nanosuit/nanosuit.obj"));
	models.emplace_back(new Model("res/models/dee/waddledee.obj"));
	models.emplace_back(new Model("res/models/grass_block/grass_block.obj"));

	Model& ourModel = *models[0];
	Model& model2 = *models[1];
	Model& model3 = *models[2];

    objects.reserve(10);
	scene.GetRoot()->SetName("Root");

	GameObject* obj = new GameObject();
	objects.emplace_back(obj);
    scene.addChild(obj);
    
    obj->components.AddComponent<Transform>();
    obj->components.GetComponent<Transform>()->setScale(glm::vec3(5.0f, 5.0f, 5.0f));
	ModelComponent* modelComponent = obj->components.AddComponent<ModelComponent>(&model2);
	modelComponent->setShader(shaders[0]);
    obj->components.AddComponent<ColliderComponent>(ColliderType::SPHERE);

    SphereCollider* sphereCollider = static_cast<SphereCollider*>(obj->components.GetComponent<ColliderComponent>()->GetColliderShape());
    sphereCollider->center = glm::vec3(-0.01f, 0.1f, 0.01f);
    sphereCollider->radius = 0.1f;
    obj->SetName("Player");
	player = obj;


	obj = new GameObject();
	objects.emplace_back(obj);
	scene.addChild(obj);

	Transform* transform = obj->components.AddComponent<Transform>();
	transform->setScale(glm::vec3(0.1f, 0.1f, 0.1f));
	transform->setTranslation(glm::vec3(2.5f, 0.0f, 0.0f));
	modelComponent = obj->components.AddComponent<ModelComponent>(&ourModel);
	modelComponent->setShader(shaders[0]);
	obj->components.AddComponent<ColliderComponent>(ColliderType::BOX);

	BoxCollider* boxCollider = static_cast<BoxCollider*>(obj->components.GetComponent<ColliderComponent>()->GetColliderShape());
	boxCollider->center = glm::vec3(0.0f, 7.7f, 0.0f);
	boxCollider->halfSize = glm::vec3(4.0f, 7.7f, 1.778f);
	obj->SetName("Nanosuit");


    obj = new GameObject();
    objects.emplace_back(obj);
    scene.addChild(obj);

	transform = obj->components.AddComponent<Transform>();
	transform->setScale(glm::vec3(5.0f, 0.1f, 5.0f));

	modelComponent = obj->components.AddComponent<ModelComponent>(&model3);
	modelComponent->setShader(shaders[0]);
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

		transform = obj->components.AddComponent<Transform>();
		transform->setScale(wallScalesAndTranslations[i].first);
		transform->setTranslation(wallScalesAndTranslations[i].second);

		modelComponent = obj->components.AddComponent<ModelComponent>(&model3);
		modelComponent->setShader(shaders[0]);
		obj->components.AddComponent<ColliderComponent>(ColliderType::BOX, true);
		obj->SetName("Wall " + std::to_string(i + 1));
	}

	obj = new GameObject();
	objects.emplace_back(obj);
	scene.addChild(obj);

	transform = obj->components.AddComponent<Transform>();
	transform->setRotation(glm::vec3(0.0f, 30.0f, 180.0f));
	transform->setTranslation(glm::vec3(1.0f, 1.0f, 2.0f));
	transform->setScale(glm::vec3(0.5f, 2.0f, 0.5f));

	modelComponent = obj->components.AddComponent<ModelComponent>(&model3);
	modelComponent->setShader(shaders[0]);
	obj->components.AddComponent<ColliderComponent>(ColliderType::BOX, true);
	obj->SetName("Wall 5");

//==============================================================================================

	glfwSetScrollCallback(window, [](GLFWwindow* window, double xoffset, double yoffset) {
		scrollXOffset += xoffset;
		scrollYOffset += yoffset;
		});

	Debug::Init(shaders[1]);


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
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS || !player)
		return;

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
	framebuffer.Bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (!show_wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }else{
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

	uint32_t width, height;
	framebuffer.GetSize(width, height);

	glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)width / (float)height, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();

	// TODO: uniform blocks
	for (Shader* shader : shaders)
	{
		shader->use();
		shader->setMat4("projection", projection);
		shader->setMat4("view", view);
	}

    scene.Draw();

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
}




// serializacja
void to_json(json& j, Transform* obj)
{
	j["translation"] = { obj->getTranslation().x, obj->getTranslation().y, obj->getTranslation().z };
	j["rotation"] = { obj->getRotation().x, obj->getRotation().y, obj->getRotation().z };
	j["scale"] = { obj->getScale().x, obj->getScale().y, obj->getScale().z };
}

void to_json(json& j, ColliderComponent* obj)
{
	j["isStatic"] = obj->isStatic;
	j["type"] = obj->GetColliderShape()->getType();

	json shapeJ;

	shapeJ["center"] = { obj->GetColliderShape()->center.x, obj->GetColliderShape()->center.y, obj->GetColliderShape()->center.z };

	if (obj->GetColliderShape()->getType() == ColliderType::BOX)
	{
		BoxCollider* box = static_cast<BoxCollider*>(obj->GetColliderShape());
		shapeJ["halfSize"] = { box->halfSize.x, box->halfSize.y, box->halfSize.z };
	}
	else if (obj->GetColliderShape()->getType() == ColliderType::SPHERE)
	{
		SphereCollider* sphere = static_cast<SphereCollider*>(obj->GetColliderShape());
		shapeJ["radius"] = sphere->radius;
	}

	j["shape"] = shapeJ;
}

void to_json(json& j, ModelComponent* obj)
{
	j["model"] = obj->GetModel().directory;
	j["shader"] = obj->getShader()->getName();
}

void to_json(json& j, GameObject* obj)
{
	j["name"] = obj->GetName();

	j["children"] = json::array();
	GameObject** children = obj->GetChildren();

	for (int i = 0; i < obj->GetChildCount(); i++)
	{
		GameObject* child = children[i];
		if (child)
		{
			j["children"].push_back(children[i]);
		}
	}

	json componentsJ;
	Transform* transform = obj->components.GetComponent<Transform>();
	if (transform)
	{
		to_json(componentsJ["transform"], transform);
	}

	ColliderComponent* collider = obj->components.GetComponent<ColliderComponent>();
	if (collider)
	{
		to_json(componentsJ["collider"], collider);
	}

	ModelComponent* model = obj->components.GetComponent<ModelComponent>();
	if (model)
	{
		to_json(componentsJ["model"], model);
	}

	j["components"] = componentsJ;
}


void serialize(const std::string& filename)
{
	json j;
	GameObject* root = scene.GetRoot();
	j["root"] = root->GetName();
	j["children"] = json::array();
	GameObject** children = root->GetChildren();

	for (int i = 0; i < root->GetChildCount(); i++)
	{
		GameObject* child = children[i];
		if (child)
		{
			j["children"].push_back(children[i]);
		}
	}

	std::ofstream file(filename);
	if (file.is_open())
	{
		file << j.dump(4);
		file.close();
	}
	else
	{
		spdlog::error("Failed to open file for serialization: {}", filename);
	}
}

// deserializacja

void from_json(const json& j, Transform*& obj)
{
	obj = new Transform();
	obj->setTranslation(glm::vec3(j["translation"][0], j["translation"][1], j["translation"][2]));
	obj->setRotation(glm::vec3(j["rotation"][0], j["rotation"][1], j["rotation"][2]));
	obj->setScale(glm::vec3(j["scale"][0], j["scale"][1], j["scale"][2]));
}

void from_json(const json& j, ColliderComponent*& obj)
{
	obj = new ColliderComponent(static_cast<ColliderType>(j["type"]));
	obj->isStatic = j["isStatic"];

	ColliderShape* shape = obj->GetColliderShape();
	shape->center = glm::vec3(j["shape"]["center"][0], j["shape"]["center"][1], j["shape"]["center"][2]);
	if (shape->getType() == ColliderType::BOX)
	{
		BoxCollider* box = static_cast<BoxCollider*>(shape);
		box->halfSize = glm::vec3(j["shape"]["halfSize"][0], j["shape"]["halfSize"][1], j["shape"]["halfSize"][2]);
	}
	else if (shape->getType() == ColliderType::SPHERE)
	{
		SphereCollider* sphere = static_cast<SphereCollider*>(shape);
		sphere->radius = j["shape"]["radius"];
	}
}

void from_json(const json& j, ModelComponent*& obj)
{
	std::string modelPath = j["model"];
	std::string shaderName = j["shader"];

	Model* objModel = nullptr;
	for (Model* model : models)
	{
		if (model->directory == modelPath)
		{
			objModel = model;
			break;
		}
	}

	if (objModel == nullptr)
	{
		spdlog::error("Model not found: {}", modelPath);
		return;
	}

	obj = new ModelComponent(objModel);

	for (Shader* shader : shaders)
	{
		if (shader->getName() == shaderName)
		{
			obj->setShader(shader);
			break;
		}
	}
}

void from_json(const json& j, GameObject*& obj)
{
	obj = new GameObject();
	obj->SetName(j["name"].get<std::string>());
	std::string name = obj->GetName();
	const json& children = j["children"];
	for (const auto& child : children)
	{
		GameObject* childObj = nullptr;
		from_json(child, childObj);
		obj->AddChild(childObj, false);
	}

	const json& components = j["components"];

	if (components.contains("transform"))
	{
		Transform* transform = nullptr;
		from_json(components["transform"], transform);
		obj->components.AddComponent(transform);
	}
	if (components.contains("collider"))
	{
		ColliderComponent* collider = nullptr;
		from_json(components["collider"], collider);
		obj->components.AddComponent(collider);
	}
	if (components.contains("model"))
	{
		ModelComponent* model = nullptr;
		from_json(components["model"], model);
		obj->components.AddComponent(model);
	}

	objects.emplace_back(obj);

	if (name == "Player")
	{
		player = obj;
	}
}

void deserialize(const std::string filename)
{
	json j;
	std::ifstream file(filename);

	if (file.is_open())
	{
		file >> j;

		file.close();
	}
	else
	{
		spdlog::error("Failed to open file for deserialization: {}", filename);
		return;
	}
	GameObject* root = scene.GetRoot();

	GameObject** children = root->GetChildren();

	for (int i = root->GetChildCount() - 1; i >= 0; i--)
	{
		GameObject* child = children[i];

		if (child)
		{
			root->RemoveChild(child);
			remove_object(child);
		}
	}
	spdlog::info("Object count before: {}", objects.size());

	root->SetName(j["root"].get<std::string>());
	const json& childrenJ = j["children"];
	for (const auto& child : childrenJ)
	{
		GameObject* childObj = nullptr;
		from_json(child, childObj);
		root->AddChild(childObj, false);
	}
	spdlog::info("Object count after: {}", objects.size());
}


void imgui_begin()
{
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void remove_object(GameObject* obj)
{
	if (obj)
	{
		objects.erase(std::remove(objects.begin(), objects.end(), obj), objects.end());
		GameObject** children = obj->GetChildren();
		for (int i = obj->GetChildCount() - 1; i >= 0; i--)
		{
			GameObject* child = children[i];
			if (child)
			{
				remove_object(child);
			}
		}
		if (obj == selectedObject)
			selectedObject = nullptr;
		if (obj == player)
			player = nullptr;

		delete obj;
		obj = nullptr;
	}
}

const char* HIERARCHY_NODE = "HIERARCHY_NODE";

void imgui_hierarchy(Scene* scene);
void imgui_hierarchy_node(GameObject* obj);
void imgui_rearrange_target(GameObject* obj, int targetIndex);

void imgui_inspector();
void imgui_transform(GameObject* obj);
void imgui_collider(GameObject* obj);
void imgui_model(GameObject* obj);

void imgui_shaders();
void imgui_uniform_descendants(const Shader& shader, const std::vector<UniformInfo>& uniforms, const std::string& fullName);
void imgui_uniform_struct(const Shader& shader, const UniformInfo& uniform, const std::string& fullName, int index);
void imgui_uniform_array(const Shader& shader, const UniformInfo& uniform, const std::string& fullName);
void imgui_uniform_leaf(const Shader& shader, const UniformInfo& uniform, const std::string& fullName, int index);

void imgui_collisions(bool& show);
void imgui_scene();

void imgui_save_load()
{
	if (ImGui::BeginPopupModal("Save/Load##Popup"))
	{
		static char filename[128] = "scene.json";
		ImGui::InputText("Filename", filename, sizeof(filename));
		if (ImGui::Button("Save"))
		{
			serialize(filename);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Load"))
		{
			deserialize(filename);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}


void imgui_render()
{
	static bool show_demo_window = false;
    static bool show_collisions = false;
	static bool show_save_load = false;

	ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("Settings"))
		{
			ImGui::MenuItem("Demo Window", NULL, &show_demo_window);
			ImGui::MenuItem("Collisions", NULL, &show_collisions);
			ImGui::MenuItem("Wireframe", NULL, &show_wireframe);
			ImGui::MenuItem("Show colliders", NULL, &show_colliders);
			
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

			ImGui::MenuItem("Save/Load", NULL, &show_save_load);

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

	if (show_save_load)
	{
		ImGui::OpenPopup("Save/Load##Popup");
		show_save_load = false;
	}
	imgui_save_load();

	
    {
		ImGui::SetNextWindowSize(ImVec2(500, 550), ImGuiCond_FirstUseEver);
        ImGui::Begin("Hello, world!");

        ImGui::End();
    }
	imgui_hierarchy(&scene);
	imgui_inspector();
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
			GameObject* root = scene->GetRoot();

			if (ImGui::BeginPopupContextWindow("Hierarchy Context Menu"))
			{
				bool disabled;
				if (disabled = root->GetChildCount() == MAX_CHILDREN) ImGui::BeginDisabled();
				if (ImGui::MenuItem("Add Object"))
				{
					GameObject* child = new GameObject();
					root->AddChild(child);
					objects.emplace_back(child);
					child->components.AddComponent<Transform>();
					child->SetName("New Object");
				}
				if (disabled) ImGui::EndDisabled();

				ImGui::EndPopup();
			}

			imgui_hierarchy_node(root);
		}
		ImGui::EndChild();
	}
	ImGui::End();
}

void imgui_hierarchy_node(GameObject* obj)
{
	if (obj == nullptr)
		return;

	ImGui::PushID(obj);
	
	std::string objName = obj->GetName();
	std::string displayName = objName + "###objName";
	int childCount = obj->GetChildCount();


	ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick
		| ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding;

	nodeFlags |= (childCount == 0) ? ImGuiTreeNodeFlags_Leaf : ImGuiTreeNodeFlags_DefaultOpen;
	if (obj == selectedObject)
		nodeFlags |= ImGuiTreeNodeFlags_Selected;


	// tree node start
	bool opened = ImGui::TreeNodeEx(displayName.c_str(), nodeFlags);

	if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
	{
		selectedObject = obj;
	}

	// Context Menu
	bool deleteObject = false;
	if (ImGui::BeginPopupContextItem("Context Menu"))
	{
		bool disabled;

		if (disabled = obj->GetChildCount() == MAX_CHILDREN) ImGui::BeginDisabled();
		if (ImGui::MenuItem("Add Child"))
		{
			GameObject* child = new GameObject();
			obj->AddChild(child);
			objects.emplace_back(child);
			child->components.AddComponent<Transform>();
			child->SetName("New Object");
		}
		if (disabled) ImGui::EndDisabled();

		if (disabled = obj == scene.GetRoot()) ImGui::BeginDisabled();
		if (ImGui::MenuItem("Delete"))
		{
			deleteObject = true;
		}
		if (ImGui::MenuItem("Duplicate") && obj->GetParent()->GetChildCount() < MAX_CHILDREN)
		{
			GameObject* duplicate = nullptr;
			json j;
			to_json(j, obj);
			from_json(j, duplicate);

			duplicate->SetName(obj->GetName() + " Copy");
			obj->GetParent()->AddChild(duplicate, false);
		}
		if (disabled) ImGui::EndDisabled();

		ImGui::EndPopup();
	}


	// Drag and Drop
	if (ImGui::BeginDragDropSource())
	{
		ImGui::SetDragDropPayload(HIERARCHY_NODE, &obj, sizeof(GameObject*));
		ImGui::Text(objName.c_str());
		ImGui::EndDragDropSource();
	}
	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(HIERARCHY_NODE))
		{
			GameObject* draggedObj = *(GameObject**)payload->Data;

			if (GameObject::ChangeParent(draggedObj, obj))
				childCount = obj->GetChildCount();
		}
		ImGui::EndDragDropTarget();
	}

	if (opened)
	{
		if (childCount > 0)
		{
			GameObject** children = obj->GetChildren();
			for (int i = 0; i < childCount; i++)
			{
				if (children[i])
				{
					imgui_rearrange_target(obj, i);
					imgui_hierarchy_node(children[i]);
				}
			}
			imgui_rearrange_target(obj, childCount);
		}

		ImGui::TreePop();
	}
	ImGui::PopID();

	if (deleteObject)
	{
		obj->GetParent()->RemoveChild(obj);
		remove_object(obj);
	}
}

void imgui_rearrange_target(GameObject* obj, int targetIndex)
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
				GameObject* draggedObj = *(GameObject**)payload->Data;

				if (draggedObj->GetParent() == obj)
				{
					int previousIndex = draggedObj->GetIndex();
					obj->SetChildIndex(previousIndex, (previousIndex < targetIndex) ? targetIndex - 1 : targetIndex);
				}
				else if (GameObject::ChangeParent(draggedObj, obj))
				{
					obj->SetChildIndex(obj->GetChildCount() - 1, targetIndex);
				}
			}
			ImGui::EndDragDropTarget();
		}
		ImGui::SetCursorPosY(cursorPosY);
	}
}

void imgui_inspector()
{
	ImGui::SetNextWindowSize(ImVec2(500, 550), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Inspector"))
	{
		if (selectedObject)
		{
			std::string objName = selectedObject->GetName();
			std::string displayName = objName + "###objName";
			const char* name = objName.c_str();

			if (ImGui::InputText("Name", (char*)name, 64))
			{
				selectedObject->SetName(name);
			}

			imgui_transform(selectedObject);
			imgui_collider(selectedObject);
			imgui_model(selectedObject);
		}
	}
	ImGui::End();
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
		default:
			break;
		}
	}


	ImGui::PopID();
}

void imgui_model(GameObject* obj)
{
	ModelComponent* modelComponent = obj->components.GetComponent<ModelComponent>();

	if (!modelComponent)
		return;

	ImGui::PushID(modelComponent);

	if (ImGui::CollapsingHeader("Model", ImGuiTreeNodeFlags_DefaultOpen))
	{
		Shader* modelShader = modelComponent->getShader();
		if (ImGui::BeginCombo("Shader##Combo", modelShader->getName().c_str()))
		{
			for (int i = 0; i < shaders.size(); i++)
			{
				if (ImGui::Selectable(shaders[i]->getName().c_str(), modelShader == shaders[i]))
				{
					modelShader = shaders[i];
					modelComponent->setShader(modelShader);
				}
			}

			ImGui::EndCombo();
		}
	}


	ImGui::PopID();
}



// Shader gui


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

