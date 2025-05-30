#include "Application.h"
#include <glad/glad.h>  // Initialize with gladLoadGL()
#include <spdlog/spdlog.h>

#include "Serialization.h"

static glm::vec4 clear_color = glm::vec4(0.45f, 0.55f, 0.60f, 1.00f);

static void glfw_error_callback(int error, const char* description)
{
	fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}


Application::Application()
{
	assert(init() && "Failed to initialize application!"); // mo�e by� assert?
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

	Serialization::loadShaderList("res/shaderList.json", shaders);

	for (Shader* shader : shaders)
	{
		GLint numUniformBlocks;
		glGetProgramiv(shader->ID, GL_ACTIVE_UNIFORM_BLOCKS, &numUniformBlocks);
		spdlog::info("Shader '{}' has {} uniform blocks.", shader->getName(), numUniformBlocks);
		for (int i = 0; i < numUniformBlocks; ++i)
		{
			GLchar blockName[256];
			glGetActiveUniformBlockName(shader->ID, i, sizeof(blockName), nullptr, blockName);
			auto it = uniformBlockMap.find(blockName);
			if (it != uniformBlockMap.end())
			{
				shader->use();
				if (!it->second->isInitialized())
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
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	setupScene();

	return true;
}


void Application::input()
{
	if (player == (EntityID)-1)
		return;


	/*auto& transform = scene->getComponent<Transform>(player);
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
	}*/
}

void Application::update()
{
	scene->getRenderingSystem().updatePreviousModelMatrices();

	auto transforms = scene->getStorage<Transform>();
	auto velocityComponents = scene->getStorage<VelocityComponent>();

	if (velocityComponents != nullptr)
	{
		for (int i = 0; i < velocityComponents->getQuantity(); i++)
		{
			auto& velocityComponent = velocityComponents->components[i];
			auto& transform = transforms->get(velocityComponent.id);
			if (transform.isStatic)
				continue;

			if (velocityComponent.useGravity)
			{
				velocityComponent.velocity.y -= 9.81f * deltaTime;
			}

			glm::vec3 newTranslation = transform.translation + velocityComponent.velocity * deltaTime;
			scene->getTransformSystem().translateEntity(velocityComponent.id, newTranslation);
			glm::vec3 newRotation = transform.eulerRotation + velocityComponent.angularVelocity * deltaTime;
			scene->getTransformSystem().rotateEntity(velocityComponent.id, newRotation);
		}
	}

	auto butterControllers = scene->getStorage<ButterController>();
	if (butterControllers != nullptr)
	{
		for (int i = 0; i < butterControllers->getQuantity(); i++)
		{
			auto& butterController = butterControllers->components[i];
			butterController.update(window, scene.get(), deltaTime);
		}
	}

	auto breadControllers = scene->getStorage<BreadController>();
	if (breadControllers != nullptr)
	{
		for (int i = 0; i < breadControllers->getQuantity(); i++)
		{
			auto& breadController = breadControllers->components[i];
			breadController.update(window, scene.get(), deltaTime);
		}
	}

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
	auto& aiSystem = scene->getFlyAISystem();
	aiSystem.deltaTime = deltaTime;
	aiSystem.update();
	ts.update();
	EventSystem& eventSystem = scene->getEventSystem();
	eventSystem.processEvents();
}




// temporary
static void lightSystem(const Scene& scene, UniformBlockStorage& uniformBlockStorage)
{
	auto& lightBlock = uniformBlockStorage.lightBlock;
	auto transforms = scene.getStorage<Transform>();

	glm::vec4 ambientColor = glm::vec4(0.3f, 0.3f, 0.3f, 1.0f); // Temporary ambient color
	lightBlock.setData("ambientColor", &ambientColor);


	// point lights
	int pointLightCount = 0;

	auto pointLights = scene.getStorage<PointLightComponent>();
	if (pointLights == nullptr)
	{
		lightBlock.setData("pointLightCount", &pointLightCount);
	}
	else
	{
		pointLightCount = pointLights->getQuantity();
		lightBlock.setData("pointLightCount", &pointLightCount);


		for (int i = 0; i < pointLightCount; i++)
		{
			auto& light = pointLights->components[i];
			std::string prefix = "pointLights[" + std::to_string(i) + "].";
			glm::vec3 position = transforms->get(light.id).globalMatrix[3];
			lightBlock.setData(prefix + "position", &position);
			lightBlock.setData(prefix + "color", &light.color);
			lightBlock.setData(prefix + "intensity", &light.intensity);
			lightBlock.setData(prefix + "constant", &light.constant);
			lightBlock.setData(prefix + "linear", &light.linear);
			lightBlock.setData(prefix + "quadratic", &light.quadratic);
		}
	}

	// directional lights
	int directionalLightCount = 0;

	auto directionalLights = scene.getStorage<DirectionalLightComponent>();
	if (directionalLights == nullptr)
	{
		lightBlock.setData("directionalLightCount", &directionalLightCount);
	}
	else
	{
		directionalLightCount = directionalLights->getQuantity();
		lightBlock.setData("directionalLightCount", &directionalLightCount);

		for (int i = 0; i < directionalLightCount; i++)
		{
			auto& light = directionalLights->components[i];
			std::string prefix = "directionalLights[" + std::to_string(i) + "].";
			glm::vec3 direction = -transforms->get(light.id).globalMatrix[2];
			lightBlock.setData(prefix + "direction", &direction);
			lightBlock.setData(prefix + "color", &light.color);
			lightBlock.setData(prefix + "intensity", &light.intensity);
		}
	}

}


void Application::render(const Framebuffer& framebuffer)
{
	framebuffer.Bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	auto& ts = scene->getTransformSystem();
	ts.update();

	//scene->getRenderingSystem().buildTree();

	lightSystem(*scene, uniformBlockStorage);

	auto transforms = scene->getStorage<Transform>();
	auto cameras = scene->getStorage<CameraComponent>();

	auto [fboWidth, fboHeight] = framebuffer.GetSizePair();
	float aspectRatio = static_cast<float>(fboWidth) / static_cast<float>(fboHeight);

	for (int i = 0; i < cameras->getQuantity(); i++)
	{
		auto& cameraComponent = cameras->components[i];
		auto& transform = transforms->get(cameraComponent.id);
		if (cameraComponent.camera.getInvViewMatrix() != transform.globalMatrix)
		{
			cameraComponent.camera.setInvViewMatrix(transform.globalMatrix);
		}
		if (cameraComponent.aspectRatio != aspectRatio)
		{
			cameraComponent.aspectRatio = aspectRatio;
			cameraComponent.dirty = true;
		}

		if (cameraComponent.dirty)
		{
			cameraComponent.updateProjectionMatrix();
		}

		scene->getRenderingSystem().drawScene(framebuffer, cameraComponent.camera, uniformBlockStorage, postShaders);
	}
}


void Application::render(Camera& camera, const Framebuffer& framebuffer)
{
	framebuffer.Bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	auto& ts = scene->getTransformSystem();
	ts.update();

	scene->getRenderingSystem().buildTree();

	lightSystem(*scene, uniformBlockStorage);

	scene->getRenderingSystem().drawScene(framebuffer, camera, uniformBlockStorage, postShaders);
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
	models.emplace_back(new Model("res/models/muhahahahahahah.fbx"));
	models.emplace_back(new Model("res/models/MASLO.fbx"));
	models.emplace_back(new Model("res/models/grass_block/grass_block.obj"));
	models.emplace_back(new Model("res/models/CHLEB.fbx"));
	models.emplace_back(new Model("res/models/poziomy.fbx"));

	Model& ourModel = *models[0];
	Model& model2 = *models[1];
	Model& model3 = *models[2];
	Model& model4 = *models[3];

	enum class FlyVariant { GREEN, RED, GOLD, PURPLE, COUNT };
	Model* flyModels[static_cast<size_t>(FlyVariant::COUNT)] =
	{
		&ourModel,	//GREEN
		&ourModel,	//RED
		&ourModel,	//GOLD
		&model4,	//PURPLE
	};
	constexpr FlyVariant SELECTED_FLY = FlyVariant::GOLD;

	scene = std::make_shared<Scene>();

	EntityID ent;
	ImageComponent* imageComponent;
	TextComponent* textComponent;
	ColliderComponent* colliderComponent;

	setupEvents();

	auto& ts = scene->getTransformSystem();


	ent = scene->getSceneRootEntity();
	scene->getComponent<ObjectInfoComponent>(ent).name = "Root";


	ent = player = scene->createEntity();
	scene->getComponent<ObjectInfoComponent>(ent).name = "Maslo";
	ts.scaleEntity(ent, glm::vec3(0.003f, 0.003f, 0.003f));
	ts.translateEntity(ent, glm::vec3(0.0f, 1.0f, 0.0f));
	scene->getComponent<Transform>(ent).isStatic = false;

	scene->addComponent<ModelComponent>(ent, { shaders[2], &model2 });

	colliderComponent = &scene->addComponent<ColliderComponent>(ent, ColliderComponent(ColliderType::BOX));
	BoxCollider* boxCollider = static_cast<BoxCollider*>(colliderComponent->GetColliderShape());
	boxCollider->halfSize = glm::vec3(163.8f, 109.3f, 87.6f);

	scene->addComponent<VelocityComponent>(ent, {});
	scene->addComponent<ButterController>(ent, { 3.0f, 5.0f });


	ent = scene->createEntity();
	scene->getComponent<ObjectInfoComponent>(ent).name = "Camera";
	auto& playerCam = scene->addComponent<CameraComponent>(ent, {});
	playerCam.camera.getFrustum().setProjectionMatrix(
		glm::perspective(glm::radians(45.0f), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 100.0f));
	ts.translateEntity(ent, glm::vec3(0.0f, 5.0f, 11.0f));
	ts.rotateEntity(ent, glm::vec3(-25.0f, 0.0f, 0.0f));

	//mucha
	ent = scene->createEntity();
	scene->getComponent<ObjectInfoComponent>(ent).name = "Fly";

	ts.scaleEntity(ent, glm::vec3(0.01f, 0.004f, 0.01f));
	ts.translateEntity(ent, glm::vec3(2.5f, 3.0f, 0.0f));
	scene->getComponent<Transform>(ent).isStatic = false;

	FlyAIComponent flySpec;
	flySpec.idButter = player;
	flySpec.diveSpeed = 3.0f;
	flySpec.diveCooldownTime = 3.0f;
	flySpec.detectionRadius = 4.0f;
	flySpec.patrolRange = 4.0f;
	flySpec.patrolSpeed = 1.5f;

	scene->addComponent<ModelComponent>(
		ent,
		{ shaders[2],
		  flyModels[static_cast<size_t>(SELECTED_FLY)] });


	colliderComponent = &scene->addComponent<ColliderComponent>(ent, ColliderComponent(ColliderType::BOX));
	boxCollider = static_cast<BoxCollider*>(colliderComponent->GetColliderShape());
	boxCollider->center = glm::vec3(0.0f, 7.7f, 0.0f);
	boxCollider->halfSize = glm::vec3(4.0f, 7.7f, 1.778f);

	scene->addComponent<FlyAIComponent>(ent, flySpec);
	//



	ent = scene->createEntity();
	scene->getComponent<ObjectInfoComponent>(ent).name = "Chleb";

	ts.scaleEntity(ent, glm::vec3(0.005f, 0.005f, 0.005f));
	ts.translateEntity(ent, glm::vec3(2.5f, 1.0f, 0.0f));
	scene->getComponent<Transform>(ent).isStatic = false;
	scene->addComponent<ModelComponent>(ent, { shaders[2], &model4 });
	colliderComponent = &scene->addComponent<ColliderComponent>(ent, ColliderComponent(ColliderType::BOX));
	boxCollider = static_cast<BoxCollider*>(colliderComponent->GetColliderShape());
	boxCollider->halfSize = glm::vec3(112.8f, 91.5f, 153.0f);

	scene->addComponent<VelocityComponent>(ent, {});
	scene->addComponent<BreadController>(ent, { 3.0f, 5.0f });

	ent = scene->createEntity();
	scene->getComponent<ObjectInfoComponent>(ent).name = "Floor";

	ts.scaleEntity(ent, glm::vec3(5.0, 0.1f, 5.0f));

	scene->addComponent<ModelComponent>(ent, { shaders[0], &model3 });

	colliderComponent = &scene->addComponent<ColliderComponent>(ent, ColliderComponent(ColliderType::BOX, true));


	std::pair<glm::vec3, glm::vec3> wallScalesAndTranslations[] = {
		//{ glm::vec3(5.0f, 1.0f, 1.0f), glm::vec3(0.0f, 1.0f, 6.0f) },
		{ glm::vec3(5.0f, 1.0f, 1.0f), glm::vec3(0.0f, 1.0f, -6.0f) },
		{ glm::vec3(1.0f, 1.0f, 5.0f), glm::vec3(6.0f, 1.0f, 0.0f) },
		{ glm::vec3(1.0f, 1.0f, 5.0f), glm::vec3(-6.0f, 1.0f, 0.0f) },
	};

	for (int i = 0; i < 3; ++i) {
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


	/*for (int x = 0; x < 100; ++x) {
		for (int z = 0; z < 10; ++z) {
			ent = scene->createEntity();
			scene->getComponent<ObjectInfoComponent>(ent).name = "Nanosuit_" + std::to_string(x) + "_" + std::to_string(z);

			ts.translateEntity(ent, glm::vec3(x * 2.0f, 0.0f, z * 2.0f));
			ts.scaleEntity(ent, glm::vec3(0.1f, 0.1f, 0.1f));

			scene->addComponent<ModelComponent>(ent, { shaders[0], &ourModel });

		}
	}*/


	scene->getTransformSystem().update();
	//scene->getRenderingSystem().buildTree();
	std::vector<Shader*> postShaderVec;
	Serialization::loadShaderList("res/postprocessShaderList.json", postShaderVec);
	for (Shader* shader : postShaderVec) {
		postShaders[shader->getName()] = shader;
	}
}


void Application::setupEvents()
{
	EventSystem& eventSystem = scene->getEventSystem();

	// reset stanu much po zaatakowaniu masła

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


	// zerowanie pionowej prędkości po dotknięciu podłoża

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


	// ponowne umożliwienie skakania po dotknięciu podłoża
	// chleb

	eventSystem.registerListener<CollisionEvent>([&](const Event& e) {
		const auto& event = static_cast<const CollisionEvent&>(e);
		if (!event.isColliding) return;

		BreadController* componentA = scene->hasComponent<BreadController>(event.objectA) ?
			&scene->getComponent<BreadController>(event.objectA) : nullptr;
		BreadController* componentB = scene->hasComponent<BreadController>(event.objectB) ?
			&scene->getComponent<BreadController>(event.objectB) : nullptr;

		if (abs(event.separationVector.y) > 0.01f)
		{
			if (componentA)
			{
				componentA->isJumping = false;
			}
			if (componentB)
			{
				componentB->isJumping = false;
			}
		}
		});

	// masło

	eventSystem.registerListener<CollisionEvent>([&](const Event& e) {
		const auto& event = static_cast<const CollisionEvent&>(e);
		if (!event.isColliding) return;

		ButterController* componentA = scene->hasComponent<ButterController>(event.objectA) ?
			&scene->getComponent<ButterController>(event.objectA) : nullptr;
		ButterController* componentB = scene->hasComponent<ButterController>(event.objectB) ?
			&scene->getComponent<ButterController>(event.objectB) : nullptr;

		if (abs(event.separationVector.y) > 0.01f)
		{
			if (componentA)
			{
				componentA->isJumping = false;
			}
			if (componentB)
			{
				componentB->isJumping = false;
			}
		}
		});


	// odbijanie się masła od chleba

	eventSystem.registerListener<CollisionEvent>([&](const Event& e) {
		const auto& event = static_cast<const CollisionEvent&>(e);
		if (!event.isColliding) return;

		ButterController* butter;
		BreadController* bread;
		if (scene->hasComponent<ButterController>(event.objectA) &&
			scene->hasComponent<BreadController>(event.objectB))
		{
			butter = &scene->getComponent<ButterController>(event.objectA);
			bread = &scene->getComponent<BreadController>(event.objectB);
		}
		else if (scene->hasComponent<BreadController>(event.objectA) &&
			scene->hasComponent<ButterController>(event.objectB))
		{
			butter = &scene->getComponent<ButterController>(event.objectB);
			bread = &scene->getComponent<BreadController>(event.objectA);
		}
		else
		{
			return;
		}

		if (bread && butter)
		{
			auto& velocityComponent = scene->getComponent<VelocityComponent>(butter->id);
			if (velocityComponent.velocity.y < 0.1f && butter->isJumping)
			{
				velocityComponent.velocity.y = butter->jumpSpeed * 1.5f;
			}
		}
		});
}