#include "Application.h"
#include <glad/glad.h>  // Initialize with gladLoadGL()
#include <spdlog/spdlog.h>

static glm::vec4 clear_color = glm::vec4(0.45f, 0.55f, 0.60f, 1.00f);

static void glfw_error_callback(int error, const char* description)
{
	fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}


Application::Application()
{
	assert(init() && "Failed to initialize application!"); // mo¿e byæ assert?
	spdlog::info("Initialized project.");
}

Application::~Application()
{
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
}


void Application::run()
{
	lastFrame = glfwGetTime();
	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		input();
		update();
		render();
		renderToWindow();
		endFrame();
	}
}



bool Application::init()
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
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
	glfwWindowHint(GLFW_MAXIMIZED, GL_TRUE);

	window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "OpenGL", nullptr, nullptr);
	if (window == nullptr)
	{
		spdlog::error("Failed to create GLFW window!");
		glfwTerminate();
		return false;
	}

	glfwMakeContextCurrent(window);
	glfwSetWindowUserPointer(window, this);

	glfwSwapInterval(1); // Enable vsync

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		spdlog::error("Failed to initialize OpenGL context!");
		return false;
	}

	glfwSetScrollCallback(window, [](GLFWwindow* window, double xoffset, double yoffset) {
		auto app = static_cast<Application*>(glfwGetWindowUserPointer(window));
		app->scrollXOffset += xoffset;
		app->scrollYOffset += yoffset;
		});

	setupScene();

	return true;
}


void Application::input()
{
	if (player == (EntityID)-1)
		return;


	auto& transform = scene->getComponent<Transform>(player);
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
		auto& ts = scene->getTransformSystem();
		ts.translateEntity(player, translation);
		ts.rotateEntity(player, rotation);
	}
}

void Application::update()
{
	auto& ts = scene->getTransformSystem();
	ts.update();

	auto& cs = scene->getCollisionSystem();
	cs.CheckCollisions();

	auto& collisions = cs.GetCollisions();

	bool updateScene = false;

	for (const CollisionEvent& collision : collisions)
	{
		auto& transformA = scene->getComponent<Transform>(collision.objectA);
		auto& transformB = scene->getComponent<Transform>(collision.objectB);

		auto& colliderA = scene->getComponent<ColliderComponent>(collision.objectA);
		auto& colliderB = scene->getComponent<ColliderComponent>(collision.objectB);

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

	EventSystem& eventSystem = scene->getEventSystem();
	eventSystem.processEvents();
}

void Application::render(const Framebuffer& framebuffer)
{
	render(camera, framebuffer);
}


void Application::render(Camera& camera, const Framebuffer& framebuffer)
{
	framebuffer.Bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	auto& ts = scene->getTransformSystem();
	ts.update();

	scene->getRenderingSystem().drawScene(framebuffer, camera);
	scene->getRenderingSystem().drawHud(framebuffer);
}

void Application::renderToWindow()
{
	int display_w, display_h;
	glfwMakeContextCurrent(window);
	glfwGetFramebufferSize(window, &display_w, &display_h);

	DefaultFramebuffer::GetInstance().Bind();
	glViewport(0, 0, display_w, display_h);
	glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
}

void Application::endFrame()
{
	scrollXOffset = scrollYOffset = 0.0f;
	glfwPollEvents();
	glfwMakeContextCurrent(window);
	glfwSwapBuffers(window);
}









void Application::setupScene()
{
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

	scene = std::make_shared<Scene>();

	EntityID ent;
	ImageComponent* imageComponent;
	TextComponent* textComponent;
	ColliderComponent* colliderComponent;

	auto& ts = scene->getTransformSystem();


	ent = scene->getSceneRootEntity();
	scene->getComponent<ObjectInfoComponent>(ent).name = "Root";


	ent = player = scene->createEntity();
	scene->getComponent<ObjectInfoComponent>(ent).name = "Player";
	ts.scaleEntity(ent, glm::vec3(5.0f, 5.0f, 5.0f));

	scene->addComponent<ModelComponent>(ent, { shaders[0], &model2 });

	colliderComponent = &scene->addComponent<ColliderComponent>(ent, ColliderComponent(ColliderType::SPHERE));
	SphereCollider* sphereCollider = static_cast<SphereCollider*>(colliderComponent->GetColliderShape());
	sphereCollider->center = glm::vec3(-0.01f, 0.1f, 0.01f);
	sphereCollider->radius = 0.1f;


	ent = scene->createEntity();
	scene->getComponent<ObjectInfoComponent>(ent).name = "Nanosuit";

	ts.scaleEntity(ent, glm::vec3(0.1f, 0.1f, 0.1f));
	ts.translateEntity(ent, glm::vec3(2.5f, 0.0f, 0.0f));

	scene->addComponent<ModelComponent>(ent, { shaders[0], &ourModel });

	colliderComponent = &scene->addComponent<ColliderComponent>(ent, ColliderComponent(ColliderType::BOX));
	BoxCollider* boxCollider = static_cast<BoxCollider*>(colliderComponent->GetColliderShape());
	boxCollider->center = glm::vec3(0.0f, 7.7f, 0.0f);
	boxCollider->halfSize = glm::vec3(4.0f, 7.7f, 1.778f);


	ent = scene->createEntity();
	scene->getComponent<ObjectInfoComponent>(ent).name = "Floor";

	ts.scaleEntity(ent, glm::vec3(5.0, 0.1f, 5.0f));

	scene->addComponent<ModelComponent>(ent, { shaders[0], &model3 });

	colliderComponent = &scene->addComponent<ColliderComponent>(ent, ColliderComponent(ColliderType::BOX, true));


	std::pair<glm::vec3, glm::vec3> wallScalesAndTranslations[] = {
		{ glm::vec3(5.0f, 1.0f, 1.0f), glm::vec3(0.0f, 1.0f, 6.0f) },
		{ glm::vec3(5.0f, 1.0f, 1.0f), glm::vec3(0.0f, 1.0f, -6.0f) },
		{ glm::vec3(1.0f, 1.0f, 5.0f), glm::vec3(6.0f, 1.0f, 0.0f) },
		{ glm::vec3(1.0f, 1.0f, 5.0f), glm::vec3(-6.0f, 1.0f, 0.0f) },
	};

	for (int i = 0; i < 4; ++i) {
		ent = scene->createEntity();
		scene->getComponent<ObjectInfoComponent>(ent).name = "Wall " + std::to_string(i + 1);

		ts.scaleEntity(ent, wallScalesAndTranslations[i].first);
		ts.translateEntity(ent, wallScalesAndTranslations[i].second);

		scene->addComponent<ModelComponent>(ent, { shaders[0], &model3 });
		colliderComponent = &scene->addComponent<ColliderComponent>(ent, ColliderComponent(ColliderType::BOX, true));
	}


	ent = scene->createEntity();
	scene->getComponent<ObjectInfoComponent>(ent).name = "Wall 5";

	ts.rotateEntity(ent, glm::vec3(0.0f, 30.0f, 180.0f));
	ts.translateEntity(ent, glm::vec3(1.0f, 1.0f, 2.0f));
	ts.scaleEntity(ent, glm::vec3(0.5f, 2.0f, 0.5f));

	scene->addComponent<ModelComponent>(ent, { shaders[0], &model3 });
	colliderComponent = &scene->addComponent<ColliderComponent>(ent, ColliderComponent(ColliderType::BOX, true));


	ent = scene->createEntity();
	scene->getComponent<ObjectInfoComponent>(ent).name = "Cloud";

	scene->addComponent(ent, ImageComponent{ shaders[3], "res/textures/cloud.png" });
	ts.translateEntity(ent, glm::vec3(9 * WINDOW_WIDTH / 10, WINDOW_HEIGHT / 10, 0.0f));
	ts.scaleEntity(ent, glm::vec3(250.0f));

	ent = scene->createEntity();
	scene->getComponent<ObjectInfoComponent>(ent).name = "Text";

	scene->addComponent(ent, TextComponent{ shaders[4], "foo", glm::vec4(1, 0, 0, 1), "text" });
	ts.translateEntity(ent, glm::vec3(1 * WINDOW_WIDTH / 10, WINDOW_HEIGHT / 10, 0.0f));


	EventSystem& eventSystem = scene->getEventSystem();
	eventSystem.registerListener<CollisionEvent>([&](const Event& e) {
		const auto& event = static_cast<const CollisionEvent&>(e);
		if (event.isColliding)
		{
			spdlog::info("Collision detected between {} and {}", event.objectA, event.objectB);
		}
		});

}