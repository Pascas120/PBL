#include "Application.h"
#include <glad/glad.h>  // Initialize with gladLoadGL()
#include <spdlog/spdlog.h>

#include "Serialization.h"
#include "ECS/components/CameraController.h"
#include "Random.h"
#include <glm/gtc/type_ptr.hpp>
#include "ECS/components/ButterController.h"

#include <fstream>
#include <random>

#include "ECS/components/BunController.h"

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
	for (Animation* animation : animations)
	{
		if (animation)
		{
			delete animation;
			animation = nullptr;
		}
	}
}

constexpr float maxDeltaTime = 1.0f / 3.0f;

void Application::run()
{
	loadScene(firstScene);

	lastFrame = glfwGetTime();
	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		deltaTime = std::min(deltaTime, maxDeltaTime);

		input();
		update();
		render(DefaultFramebuffer::GetInstance());
		renderToWindow();
		endFrame();
	}
}


static std::array<glm::vec3, 64> generateSSAOKernel() {
	std::array<glm::vec3, 64> kernel;
	for (int i = 0; i < 64; ++i) {
		glm::vec3 sample = Random::inUnitSphere();
		sample.z = std::abs(sample.z);
		sample *= Random::getFloat(0.0f, 1.0f);

		float scale = float(i) / 64.0f;
		scale = glm::mix(0.1f, 1.0f, scale * scale);
		sample *= scale;

		kernel[i] = sample;
	}
	std::reverse(kernel.begin(), kernel.end());
	return kernel;
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

	ma_result result = ma_engine_init(nullptr, &audioEngine);
	if (result != MA_SUCCESS) {
		spdlog::error("AudioSystem: Failed to initialize audio engine: {}", ma_result_description(result));
		return false;
	}

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


	auto bindBlocks = [&](Shader* shader)
		{
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
		};

	Serialization::loadShaderList("res/shaderList.json", shaders);
	for (Shader* shader : shaders)
	{
		bindBlocks(shader);
	}
	std::vector<Shader*> postShaderVec;
	Serialization::loadShaderList("res/postprocessShaderList.json", postShaderVec);
	for (Shader* shader : postShaderVec) {
		bindBlocks(shader);
		postShaders[shader->getName()] = shader;
	}
	std::array<glm::vec3, 64> ssaoKernel = generateSSAOKernel();
	postShaders["SSAO"]->use();
	glUniform3fv(glGetUniformLocation(postShaders["SSAO"]->ID, "samples"), ssaoKernel.size(), glm::value_ptr(ssaoKernel[0]));
		

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// TODO?: osobne pliki dla każdego prefabu
	std::ifstream file("res/prefabs.json");
	if (file.is_open())
	{
		json prefabListJson;
		file >> prefabListJson;
		file.close();

		for (const auto& prefabJson : prefabListJson["prefabs"])
		{
			std::string prefabName = prefabJson["name"].get<std::string>();
			prefabs[prefabName] = prefabJson["data"];
		}
	}
	//setupScene();
	models.emplace_back(new Model("res/models/MASLOmesh+bones.fbx"));
	models.emplace_back(new Model("res/models/chlebekmesh+bones.fbx"));
	models.emplace_back(new Model("res/models/GABKA.fbx"));
	models.emplace_back(new Model("res/models/grass_block/grass_block.obj"));
	models.emplace_back(new Model("res/models/poziomy.fbx"));
	models.emplace_back(new Model("res/models/wallpaper.fbx"));
	models.emplace_back(new Model("res/models/blat.fbx"));
	models.emplace_back(new Model("res/models/mikrofala.fbx"));
	models.emplace_back(new Model("res/models/plyta.fbx"));
	models.emplace_back(new Model("res/models/zlew_blat.fbx"));
	models.emplace_back(new Model("res/models/woda.fbx"));
	models.emplace_back(new Model("res/models/plama1.fbx"));
	models.emplace_back(new Model("res/models/plama2.fbx"));
	models.emplace_back(new Model("res/models/plama3.fbx"));
	models.emplace_back(new Model("res/models/szuflada.fbx"));
	models.emplace_back(new Model("res/models/szafkaszufladowa.fbx"));
	models.emplace_back(new Model("res/models/samblatdowindy.fbx"));
	models.emplace_back(new Model("res/models/koszdol.fbx"));
	models.emplace_back(new Model("res/models/koszgora.fbx"));
	models.emplace_back(new Model("res/models/mikrofalapoprawka.fbx"));
	models.emplace_back(new Model("res/models/STOL.fbx"));
	models.emplace_back(new Model("res/models/KRZESLO.fbx"));
	models.emplace_back(new Model("res/models/ksiazkakucharska.fbx"));
	models.emplace_back(new Model("res/models/ksiazka.fbx"));
	models.emplace_back(new Model("res/models/ksiazka2.fbx"));
	models.emplace_back(new Model("res/models/talerz.fbx"));
	models.emplace_back(new Model("res/models/talerze.fbx"));
	models.emplace_back(new Model("res/models/pomidor.fbx"));
	models.emplace_back(new Model("res/models/SER.fbx"));
	models.emplace_back(new Model("res/models/guzikM.fbx"));
	models.emplace_back(new Model("res/models/guzikD.fbx"));
	models.emplace_back(new Model("res/models/guziklodowkacaly.fbx"));
	models.emplace_back(new Model("res/models/guziklodowkadol.fbx"));
	models.emplace_back(new Model("res/models/guziklodowkagora.fbx"));
	models.emplace_back(new Model("res/models/PODLOGA.fbx"));
	models.emplace_back(new Model("res/models/kubek1.fbx"));
	models.emplace_back(new Model("res/models/kubek2.fbx"));
	models.emplace_back(new Model("res/models/kubek3.fbx"));
	models.emplace_back(new Model("res/models/kubek4.fbx"));
	models.emplace_back(new Model("res/models/kubek5.fbx"));
	models.emplace_back(new Model("res/models/kubek6.fbx"));
	models.emplace_back(new Model("res/models/musztardaplama.fbx"));
	models.emplace_back(new Model("res/models/ketchupplama.fbx"));
	models.emplace_back(new Model("res/models/ketchup.fbx"));
	models.emplace_back(new Model("res/models/mustard.fbx"));
	models.emplace_back(new Model("res/models/lody1.fbx"));
	models.emplace_back(new Model("res/models/lody2.fbx"));
	models.emplace_back(new Model("res/models/znajdzka.fbx"));
	models.emplace_back(new Model("res/models/lodowkacialo.fbx"));
	models.emplace_back(new Model("res/models/polkacalalewo.fbx"));
	models.emplace_back(new Model("res/models/polkacalaprawo.fbx"));
	models.emplace_back(new Model("res/models/polkacalabez.fbx"));
	models.emplace_back(new Model("res/models/polkalewowdolprawo.fbx"));
	models.emplace_back(new Model("res/models/polkalewowdolbez.fbx"));
	models.emplace_back(new Model("res/models/polkalewowdollewo.fbx"));
	models.emplace_back(new Model("res/models/polkaprawowdollewo.fbx"));
	models.emplace_back(new Model("res/models/polkaprawowdolprawo.fbx"));
	models.emplace_back(new Model("res/models/polkaprawowdolbez.fbx"));
	models.emplace_back(new Model("res/models/polkasmol.fbx"));
	models.emplace_back(new Model("res/models/butelki.fbx"));
	models.emplace_back(new Model("res/models/lodowkadziura.fbx"));
	models.emplace_back(new Model("res/models/kratka.fbx"));
	models.emplace_back(new Model("res/models/sciana.fbx"));

	models.emplace_back(new Model("res/models/puszka.fbx"));
	models.emplace_back(new Model("res/models/mleczko.fbx"));
	models.emplace_back(new Model("res/models/jogurt.fbx"));
	models.emplace_back(new Model("res/models/butter.fbx"));
	models.emplace_back(new Model("res/models/jajka.fbx"));

	//animacje
	animations.emplace_back(new Animation("res/anims/chlebekchod.fbx", models[1]));
	animations.emplace_back(new Animation("res/anims/chlebekdown.fbx", models[1]));
	animations.emplace_back(new Animation("res/anims/chlebekup.fbx", models[1]));
	animations.emplace_back(new Animation("res/anims/maselkochod.fbx", models[0]));
	animations.emplace_back(new Animation("res/anims/maselkodown.fbx", models[0]));
	animations.emplace_back(new Animation("res/anims/maselkoup.fbx", models[0]));
	animations.emplace_back(new Animation("res/anims/GABKAfloat.fbx", models[2]));

	//TODO Automatyczne wczytywanie z folderu
	sounds.emplace_back("res/sounds/background.mp3");
	sounds.emplace_back("res/sounds/boing.mp3");
	sounds.emplace_back("res/sounds/ost1.mp3");
	sounds.emplace_back("res/sounds/ost2.mp3");
	sounds.emplace_back("res/sounds/ost3.mp3");
	sounds.emplace_back("res/sounds/ost4.mp3");
	sounds.emplace_back("res/sounds/ost5.mp3");
	sounds.emplace_back("res/sounds/cieplaStrefaMasloRoztapianie.mp3");
	sounds.emplace_back("res/sounds/DuzyPrzycisk.wav");
	sounds.emplace_back("res/sounds/malyPrzycisk.mp3");
	sounds.emplace_back("res/sounds/poslizgchlebaodmasla.wav");
	sounds.emplace_back("res/sounds/skokChleb.mp3");
	sounds.emplace_back("res/sounds/skokMaslo.mp3");
	sounds.emplace_back("res/sounds/SzafkaDrzwiOtwieranie.mp3");
	sounds.emplace_back("res/sounds/SzafkaDrzwiZamykanie.mp3");
	sounds.emplace_back("res/sounds/trampolinaSound.mp3");
	sounds.emplace_back("res/sounds/UISelectionSound.wav");
	sounds.emplace_back("res/sounds/winda.mp3");
	sounds.emplace_back("res/sounds/track_1.mp3");
	sounds.emplace_back("res/sounds/track_2.mp3");
	sounds.emplace_back("res/sounds/track_3.mp3");
	sounds.emplace_back("res/sounds/track_4.mp3");
	sounds.emplace_back("res/sounds/znajdzka.wav");
	sounds.emplace_back("res/sounds/otwieranieBulki.wav");
	sounds.emplace_back("res/sounds/zart.wav");

	std::ifstream fileJokes("res/jokes.txt");
	if (!fileJokes.is_open())
	{
		spdlog::error("Failed to open jokes file!");
		return false;
	}
	for (std::string line; std::getline(fileJokes, line);)
	{
		if (!line.empty())
		{
			jokes.push_back(line);
		}
	}
	file.close();
	std::shuffle(jokes.begin() + 1, jokes.end(), std::mt19937(std::random_device()()));
	spdlog::info("Loaded {} jokes.", jokes.size());
	spdlog::info(jokes[0]);


	// TODO: przenieść poza Application.cpp ( nie zrobimy tego ;) )
	file.open("res/dialogues.json");
	if (!file.is_open())
	{
		spdlog::error("Failed to open dialogue file!");
		return false;
	}
	json dialogueListJson;
	file >> dialogueListJson;
	file.close();

	for (json dialogueJson : dialogueListJson["dialogues"])
	{
		std::vector<DialogueBox> dialogue;
		for (json dialogueSpeechJson : dialogueJson["lines"])
		{
			DialogueBox box;
			box.character = dialogueSpeechJson["character"];

			json linesJson = dialogueSpeechJson["textLines"];
			for (size_t i = 0; i < linesJson.size(); i++)
			{
				box.lines[i] = linesJson[i];
			}

			dialogue.push_back(box);
		}

		dialogues[dialogueJson["dialogueName"]] = dialogue;
	}

	dialogueCharacterInfo["chleb"] = {
		.name = "CHLEB",
		.imagePath = "res/textures/dialog_chleb.png",
	};
	dialogueCharacterInfo["maslo"] = {
		.name = "MASLO",
		.imagePath = "res/textures/dialog_maslo.png",
	};
	dialogueCharacterInfo["pomidor"] = {
		.name = "POMIDOR",
		.imagePath = "res/textures/dialog_pomidor.png",
	};
	dialogueCharacterInfo["ser"] = {
		.name = "SER",
		.imagePath = "res/textures/dialog_ser.png",
	};


	return true;
}


void Application::input()
{
	if (player == (EntityID)-1)
		return;

}

void Application::update()
{
	scene->getRenderingSystem().updatePreviousModelMatrices();

	auto& ts = scene->getTransformSystem();

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

			//sprint z trail
			if (scene->hasComponent<TrailCollisionDetectorComponent>(velocityComponent.id))
			{
				const auto& det = scene->getComponent<TrailCollisionDetectorComponent>(velocityComponent.id);
				if (det.sprintTimeLeft > 0.f)
				{
					velocityComponent.velocity.x *= det.sprintMultiplier;
					velocityComponent.velocity.z *= det.sprintMultiplier;
				}
			}

			glm::vec3 newTranslation = transform.translation + velocityComponent.velocity * deltaTime;
			ts.translateEntity(velocityComponent.id, newTranslation);
			glm::vec3 newRotation = transform.eulerRotation + velocityComponent.angularVelocity * deltaTime;
			ts.rotateEntity(velocityComponent.id, newRotation);
		}
	}

	// trail sprint
	if (auto dets = scene->getStorage<TrailCollisionDetectorComponent>())
	{
		for (int i = 0; i < dets->getQuantity(); ++i)
		{
			auto& det = dets->components[i];
			if (det.sprintTimeLeft > 0.f)
				det.sprintTimeLeft -= deltaTime;
		}
	}


	auto butterControllers = scene->getStorage<ButterController>();
	if (butterControllers != nullptr)
	{
		for (int i = 0; i < butterControllers->getQuantity(); i++)
		{
			auto& butterController = butterControllers->components[i];
			if(scene->playerLock && scene->hasComponent<VelocityComponent>(butterController.id)) {
				scene->getComponent<VelocityComponent>(butterController.id).velocity.x = 0.0f;
				scene->getComponent<VelocityComponent>(butterController.id).velocity.z = 0.0f;
				continue;
			}
			butterController.update(window, scene.get(), deltaTime);
		}
	}

	auto breadControllers = scene->getStorage<BreadController>();
	if (breadControllers != nullptr)
	{
		for (int i = 0; i < breadControllers->getQuantity(); i++)
		{
			auto& breadController = breadControllers->components[i];
			if(scene->playerLock && scene->hasComponent<VelocityComponent>(breadController.id)) {
				scene->getComponent<VelocityComponent>(breadController.id).velocity.x = 0.0f;
				scene->getComponent<VelocityComponent>(breadController.id).velocity.z = 0.0f;
				continue;
			}

			breadController.update(window, scene.get(), deltaTime);
		}
	}

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

		if ((colliderA.isStatic && colliderB.isStatic) ||
			(colliderA.isTrigger || colliderB.isTrigger))
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

	if (updateScene) {
		ts.update();
		scene->getAnimationSystem().update(deltaTime);
	}
	std::unordered_set<EntityID> pressedButtons;
	auto isPlayer = [&](EntityID id) {
		return scene->hasComponent<ButterController>(id) ||
			scene->hasComponent<BreadController>(id);
		};
	auto isButton = [&](EntityID id) {
		return scene->hasComponent<ButtonComponent>(id);
		};
	for (auto& col : collisions)
	{
		// ignorujemy kolizje, w których nie ma żadnego przycisku
		if (!isButton(col.objectA) && !isButton(col.objectB)) continue;

		EntityID btnId = isButton(col.objectA) ? col.objectA : col.objectB;
		EntityID otherId = (btnId == col.objectA) ? col.objectB : col.objectA;

		auto& btn = scene->getComponent<ButtonComponent>(btnId);
		bool   ok = false;

		switch (btn.activatorMode)
		{
		case ButtonComponent::ActivatorMode::PlayersOnly:
			ok = scene->hasComponent<ButterController>(otherId) ||
				scene->hasComponent<BreadController>(otherId);
			break;

		case ButtonComponent::ActivatorMode::AllByTag:
			if (btn.activatorTag.empty())             // brak filtru -> wszystko
				ok = true;
			else if (scene->hasComponent<ObjectInfoComponent>(otherId))
				ok = (scene->getComponent<ObjectInfoComponent>(otherId).tag == btn.activatorTag);
			break;
		}

		if (!ok) continue;

		// sprawdzamy czy faktycznie stoi na przycisku (stary warunek)
		if (std::abs(col.separationVector.x) < 0.001f &&
			std::abs(col.separationVector.z) < 0.001f)
		{
			pressedButtons.insert(btnId);
		}
	}


	auto isButtonPressed = [&](const ButtonComponent& b) -> bool
		{
			bool pressedSelf = pressedButtons.count(b.id) > 0;

			if (b.linkMode == ButtonLinkMode::DoubleD &&
				b.linkedButton != static_cast<EntityID>(-1))
			{
				bool pressedLinked = pressedButtons.count(b.linkedButton) > 0;
				return pressedSelf && pressedLinked;   
			}
			return pressedSelf;                       
		};
	if (auto buttons = scene->getStorage<ButtonComponent>()) {
		for (int i = 0; i < buttons->getQuantity(); ++i) {
			auto& btn = buttons->components[i];
			if (btn.elevatorEntity == (EntityID)-1) continue;
			auto& e = scene->getComponent<ElevatorComponent>(btn.elevatorEntity);

			bool nowPressed = isButtonPressed(btn);

			if (e.rotate)            // rotate
			{
				if (nowPressed && e.state != ElevatorState::Opening && e.state != ElevatorState::Open) {
					e.state = ElevatorState::Opening;
					e.isMoving = true;
					scene->getAudioSystem().playSound("res/sounds/DuzyPrzycisk.wav");
				}
				else if (!nowPressed && e.state != ElevatorState::Closing && e.state != ElevatorState::Closed) {
					e.state = ElevatorState::Closing;
					e.isMoving = true;
					scene->getAudioSystem().playSound("res/sounds/DuzyPrzycisk.wav");
				}
			}
			else if (e.isDoor)       //drzwi
			{
				if (e.locked) {
					if (nowPressed && e.state == ElevatorState::Closed) {
						e.state = ElevatorState::Opening;
						e.isMoving = true;
						scene->getAudioSystem().playSound("res/sounds/DuzyPrzycisk.wav");
					}
					else if (nowPressed && e.state == ElevatorState::Open) {
						e.state = ElevatorState::Closing;
						e.isMoving = true;
						scene->getAudioSystem().playSound("res/sounds/DuzyPrzycisk.wav");
					}
				}
				else {
					if (nowPressed && e.state != ElevatorState::Opening && e.state != ElevatorState::Open) {
						e.state = ElevatorState::Opening;
						e.isMoving = true;
						scene->getAudioSystem().playSound("res/sounds/DuzyPrzycisk.wav");
					}
					else if (!nowPressed && e.state != ElevatorState::Closing && e.state != ElevatorState::Closed) {
						e.state = ElevatorState::Closing;
						e.isMoving = true;
						scene->getAudioSystem().playSound("res/sounds/DuzyPrzycisk.wav");
					}
				}
			}
			else                      //winda
			{
				if (nowPressed && e.state != ElevatorState::Opening && e.state != ElevatorState::Open) {
					e.state = ElevatorState::Opening;
					e.isMoving = true;
					scene->getAudioSystem().playSound("res/sounds/DuzyPrzycisk.wav");
				}
				else if (!nowPressed && e.state != ElevatorState::Closing && e.state != ElevatorState::Closed) {
					e.state = ElevatorState::Closing;
					e.isMoving = true;
					scene->getAudioSystem().playSound("res/sounds/DuzyPrzycisk.wav");
				}
			}
		}
	}

	if (auto elevs = scene->getStorage<ElevatorComponent>()) {
		for (int i = 0; i < elevs->getQuantity(); ++i) {
			auto& e = elevs->components[i];
			if (!e.isMoving) continue;

			auto& tr = scene->getComponent<Transform>(e.id);


			if (!e.hasInitClosedPos) {
				e.closedPos = tr.translation;
				e.hasInitClosedPos = true;
			}

			float delta = e.speed * deltaTime;

			if (e.rotate) {

			
				if (!e.hasInitClosedPos) {
					e.closedPos = tr.translation;
					
					switch (e.rotateAxis) {
					case ElevatorComponent::RotateAxis::X: e.startAngle = tr.eulerRotation.x; break;
					case ElevatorComponent::RotateAxis::Y: e.startAngle = tr.eulerRotation.y; break;
					case ElevatorComponent::RotateAxis::Z: e.startAngle = tr.eulerRotation.z; break;
					}
					e.hasInitClosedPos = true;
				}

				float& current = (e.rotateAxis == ElevatorComponent::RotateAxis::X) ? tr.eulerRotation.x
					: (e.rotateAxis == ElevatorComponent::RotateAxis::Y) ? tr.eulerRotation.y
					: tr.eulerRotation.z;

				float target = e.startAngle + e.rotateAngle;   

				if (e.state == ElevatorState::Opening) {
					current += delta;
					if (current >= target) {
						current = target;
						e.state = ElevatorState::Open;
						e.isMoving = false;
					}
				}
				else if (e.state == ElevatorState::Closing) {
					current -= delta;
					if (current <= e.startAngle) {
						current = e.startAngle;
						e.state = ElevatorState::Closed;
						e.isMoving = false;
					}
				}

				
				tr.rotation = glm::quat(glm::radians(tr.eulerRotation));
				tr.isDirty = true;
				ts.rotateEntity(e.id, tr.eulerRotation);

				continue;  
			}

			if (e.isDoor) {

				float dir = (e.doorDir == ElevatorComponent::DoorDir::Left ? -1.0f : +1.0f);
				float minX = e.closedPos.x;
				float maxX = e.closedPos.x + e.openHeight * dir;

				if (e.state == ElevatorState::Opening) {
					tr.translation.x += delta * dir;
					if ((dir > 0 && tr.translation.x >= maxX) ||
						(dir < 0 && tr.translation.x <= maxX))
					{
						tr.translation.x = maxX;
						e.state = ElevatorState::Open;
						e.isMoving = false;
						spdlog::info("Door opened!");
					}
				}
				else if (e.state == ElevatorState::Closing) {
					tr.translation.x -= delta * dir;
					if ((dir > 0 && tr.translation.x <= minX) ||
						(dir < 0 && tr.translation.x >= minX))
					{
						tr.translation.x = minX;
						e.state = ElevatorState::Closed;
						e.isMoving = false;
						spdlog::info("Door closed!");
					}
				}
			}
			else {

				float minY = e.closedPos.y;
				float maxY = e.closedPos.y + e.openHeight;

				if (e.state == ElevatorState::Opening) {
					tr.translation.y += delta;
					if (tr.translation.y >= maxY) {
						tr.translation.y = maxY;
						e.state = ElevatorState::Open;
						e.isMoving = false;
						spdlog::info("Elevator ruszyla");
					}
				}
				else if (e.state == ElevatorState::Closing) {
					tr.translation.y -= delta;
					if (tr.translation.y <= minY) {
						tr.translation.y = minY;
						e.state = ElevatorState::Closed;
						e.isMoving = false;
						spdlog::info("Elevator zastopowala");
					}
				}
			}

			ts.translateEntity(e.id, tr.translation);
		}
	}

	{
		auto& aiSystem = scene->getFlyAISystem();
		aiSystem.deltaTime = deltaTime;
		aiSystem.update();
	}
	ts.update();

	if (auto bhs = scene->getStorage<ButterHealthComponent>()) {
		auto transforms = scene->getStorage<Transform>();
		for (int i = 0; i < bhs->getQuantity(); ++i) {
			auto& bh = bhs->components[i];
			auto& tr = transforms->get(bh.id);


			if (bh.burning && bh.timeLeft > 0.0f)
				bh.timeLeft -= deltaTime;

			if (bh.healing && bh.timeLeft < bh.secondsToDie)
				bh.timeLeft += deltaTime * (bh.secondsToDie / bh.secondsToHeal);


			bh.timeLeft = glm::clamp(bh.timeLeft, 0.0f, bh.secondsToDie);


			float lostRatio = 1.0f - (bh.timeLeft / bh.secondsToDie);
			float scaleRatio = glm::mix(1.0f, bh.minScale, lostRatio);
			ts.scaleEntity(bh.id, bh.startScale * scaleRatio);


			bh.burning = bh.healing = false;
		}

		ts.update();
	}

	if (auto cc = scene->getStorage<CameraController>())
	{
		for (int i = 0; i < cc->getQuantity(); ++i)
		{
			auto& controller = cc->components[i];
			controller.update(window, scene.get(), deltaTime);
		}
	}
	if (auto ssc = scene->getStorage<SplitScreenController>())
	{
		for (int i = 0; i < ssc->getQuantity(); ++i)
		{
			auto& controller = ssc->components[i];
			controller.update(window, scene.get(), deltaTime);
		}
	}

	if (auto levelExits = scene->getStorage<LevelExitComponent>())
	{
		for (int i = 0; i < levelExits->getQuantity(); ++i)
		{
			auto& levelExit = levelExits->components[i];
			levelExit.playerCount = 0;
		}
	}


	if (auto bun = scene->getStorage<BunController>())
	{
		for (int i = 0; i < bun->getQuantity(); ++i)
		{
			auto& controller = bun->components[i];
			controller.update(window, scene.get(), deltaTime);
		}
	}

	if (auto collectibles = scene->getStorage<CollectibleComponent>())
	{
		constexpr float collectibleOscTime = 4.0f;
		constexpr float collectibleOscAmplitude = 0.3f;

		for (int i = 0; i < collectibles->getQuantity(); ++i)
		{
			auto& collectible = collectibles->components[i];

			collectible.oscillationTimer += deltaTime;
			collectible.oscillationTimer = fmodf(collectible.oscillationTimer, collectibleOscTime);

			float heightOffset = sinf(collectible.oscillationTimer / collectibleOscTime * glm::two_pi<float>());
			heightOffset *= collectibleOscAmplitude;

			glm::vec3 newPos = collectible.startPosition;
			newPos.y += heightOffset;

			ts.translateEntity(collectible.id, newPos);
		}
	}

	GLFWgamepadstate state;
	glfwGetGamepadState(GLFW_JOYSTICK_1, &state);

	bool advanceButton = glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) ||
		state.buttons[GLFW_GAMEPAD_BUTTON_A] ||
		state.buttons[GLFW_GAMEPAD_BUTTON_B] ||
		state.buttons[GLFW_GAMEPAD_BUTTON_X] ||
		state.buttons[GLFW_GAMEPAD_BUTTON_Y];

	// wyświetlanie dialogów
	if (!currentDialogue.empty())
	{
		if (dialogueBoxEntities.background == (EntityID)-1)
		{
			dialogueBoxEntities.background = scene->instantiatePrefab("Dialog")[0];
			auto& dialogueBgChildren = scene->getComponent<Transform>(dialogueBoxEntities.background).children;
			for (auto& child : dialogueBgChildren)
			{
				std::string& childName = scene->getComponent<ObjectInfoComponent>(child).name;
				if (childName == "Obrazek")
				{
					dialogueBoxEntities.icon = child;
				}
				else if (childName == "Imie")
				{
					dialogueBoxEntities.name = child;
				}
				else if (childName == "Imie Cien")
				{
					dialogueBoxEntities.nameShadow = child;
				}
				else if (childName.starts_with("Linia") && isdigit(childName.back()))
				{
					dialogueBoxEntities.lines[childName.back() - '0' - 1] = child;
				}
			}
		}

		if (updateDialogueBox)
		{
			DialogueBox& currentBox = currentDialogue.front();
			auto it = dialogueCharacterInfo.find(currentBox.character);
			if (it != dialogueCharacterInfo.end())
			{

				DialogueCharacter& characterInfo = it->second;
				scene->getComponent<ImageComponent>(dialogueBoxEntities.icon).texturePath = characterInfo.imagePath;
				scene->getComponent<TextComponent>(dialogueBoxEntities.name).text = characterInfo.name;
				scene->getComponent<TextComponent>(dialogueBoxEntities.nameShadow).text = characterInfo.name;
				for (int i = 0; i < currentBox.lines.size(); i++)
				{
					scene->getComponent<TextComponent>(dialogueBoxEntities.lines[i]).text = currentBox.lines[i];
				}
			}
			updateDialogueBox = false;
		}

		if (!dialogueAdvanceKeyPressed && advanceButton)
		{
			currentDialogue.pop();
			updateDialogueBox = true;
			if (currentDialogue.empty())
			{
				scene->destroyEntity(dialogueBoxEntities.background);
				dialogueBoxEntities = {};
				scene->playerLock = false;
			}
			dialogueAdvanceKeyPressed = true;
		}
	}
	if (!advanceButton)
	{
		dialogueAdvanceKeyPressed = false;
	}

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

	auto& renderingSystem = scene->getRenderingSystem();

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


	}
	auto splitScreenControllers = scene->getStorage<SplitScreenController>();

	if (cameras->getQuantity() > 1 && splitScreenControllers != nullptr && splitScreenControllers->getQuantity() >= 1)
	{
		auto& ssc = splitScreenControllers->components[0];
		auto& cam1 = scene->getComponent<CameraComponent>(ssc.camera1);
		glm::vec3& p1 = scene->getComponent<Transform>(ssc.target1).translation;

		auto& cam2 = scene->getComponent<CameraComponent>(ssc.camera2);
		glm::vec3& p2 = scene->getComponent<Transform>(ssc.target2).translation;



		float split_threshold = 8.0f;

		if (ssc.splitActive)
		{
			glm::vec2 world_p1 = glm::vec2(p1.x, p1.z);
			glm::vec2 world_p2 = glm::vec2(p2.x, p2.z);

			glm::vec2 dx = world_p2 - world_p1;
			glm::vec2 center = (world_p1 + world_p2) / 2.0f;

			float splitSlope;

			if (dx.y == 0.0f)
			{
				splitSlope = 100000.0f;
			}
			else
			{
				splitSlope = -dx.x / dx.y;
			}

			uniformBlockStorage.splitScreenBlock.setData("split_slope", &ssc.splitSlope);

			constexpr glm::vec4 split_line_color = { 0.0f, 0.0f, 0.0f, 1.0f };
			uniformBlockStorage.splitScreenBlock.setData("split_line_color", &split_line_color);

			bool player1AboveSlope = (p1.z - (splitSlope * (p1.x - center.x) + center.y)) < 0.0f;

			int target1AboveSlope = ssc.target1AboveSlope;
			uniformBlockStorage.splitScreenBlock.setData("player1_above", &target1AboveSlope);

			uniformBlockStorage.splitScreenBlock.setData("split_line_thickness", &ssc.splitLineThickness);



			renderingSystem.drawScene(framebuffer, cam1.camera, &cam2.camera, uniformBlockStorage, postShaders);
		}
		else
		{
			cam1.screenOffset = glm::vec2(0.0f, 0.0f);
			cam1.updateProjectionMatrix();
			renderingSystem.drawScene(framebuffer, cam1.camera, nullptr, uniformBlockStorage, postShaders);
		}
	}
	else
	{
		renderingSystem.drawScene(framebuffer, cameras->components[0].camera, nullptr, uniformBlockStorage, postShaders);
	}
	renderingSystem.drawHud(framebuffer, postShaders);
}


void Application::render(Camera& camera, const Framebuffer& framebuffer)
{
	framebuffer.Bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	auto& ts = scene->getTransformSystem();
	ts.update();

	//scene->getRenderingSystem().buildTree();

	lightSystem(*scene, uniformBlockStorage);

	scene->getRenderingSystem().drawScene(framebuffer, camera, nullptr, uniformBlockStorage, postShaders);
	scene->getRenderingSystem().drawHud(framebuffer, postShaders);
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

	if (!changeSceneTo.empty() && currentDialogue.empty())
	{
		loadScene(changeSceneTo);
		changeSceneTo.clear();
	}
}

void Application::loadScene(const std::string path)
{
	scene = std::make_shared<Scene>(this);
	Serialization::loadScene(path, *scene, { shaders, models,  sounds, animations, true });
	scenePath = path;


	setupEvents();
	scene->getTransformSystem().update();
	scene->getRenderingSystem().buildTree();
	scene->getCollisionSystem().buildTree();
	setStartValues();
}



std::vector<EntityID> Application::instantiatePrefab(const std::string& prefabName, Scene& scene, EntityID parent)
{
	if (parent == (EntityID)-1)
	{
		parent = scene.getSceneRootEntity();
	}

	auto it = prefabs.find(prefabName);
	if (it == prefabs.end())
	{
		return {};
	}

	json prefabData = it->second;
	std::vector<EntityID> instantiatedEntities = Serialization::deserializeObjects(
		prefabData, scene, parent, { shaders, models, sounds, animations, false });

	return instantiatedEntities;
}







void Application::setupScene()
{
	

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

	scene = std::make_shared<Scene>(this);

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
	auto& bh = scene->addComponent<ButterHealthComponent>(ent, {});
	bh.startScale = scene->getComponent<Transform>(ent).scale;
	scene->getComponent<ObjectInfoComponent>(ent).tag = "maslo";





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

	scene->addComponent(ent, ImageComponent{ "res/textures/cloud.png" });
	ts.translateEntity(ent, glm::vec3(9 * WINDOW_WIDTH / 10, WINDOW_HEIGHT / 10, 0.0f));
	ts.scaleEntity(ent, glm::vec3(250.0f));

	ent = scene->createEntity();
	scene->getComponent<ObjectInfoComponent>(ent).name = "Text";

	scene->addComponent(ent, TextComponent{  "foo", glm::vec4(1, 0, 0, 1), "text" });
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
}


void Application::setupEvents()
{
	EventSystem& eventSystem = scene->getEventSystem();

	// reset stanu much po zaatakowaniu masła

	eventSystem.registerListener<CollisionEvent>([&](const Event& e) {
		const auto& event = static_cast<const CollisionEvent&>(e);
		if (!event.isColliding) return;

		if (scene->hasComponent<FlyAIComponent>(event.objectA) &&
			scene->hasComponent<ButterController>(event.objectB))
		{
			spdlog::info("mucha uderzyla!({} vs {})", event.objectA, event.objectB);
			FlyAIComponent& fly = scene->getComponent<FlyAIComponent>(event.objectA);
			fly.diveCooldownTimer = fly.diveCooldownTime;
			fly.state = fly.Returning;
		}
	});


	// zerowanie pionowej prędkości po dotknięciu podłoża

	eventSystem.registerListener<CollisionEvent>([&](const Event& e) {
		const auto& event = static_cast<const CollisionEvent&>(e);
		if (!event.isColliding) return;

		if (!scene->hasComponent<VelocityComponent>(event.objectA))
			return;

		VelocityComponent* velocityComponent = &scene->getComponent<VelocityComponent>(event.objectA);

		if (velocityComponent->useGravity && glm::length(event.separationVector) != 0.0f)
		{
			float upDot = glm::dot(glm::normalize(event.separationVector), glm::vec3(0.0f, 1.0f, 0.0f));
			if ((upDot > 0.9f && velocityComponent->velocity.y < 0.1f) ||
				(upDot < -0.9f && velocityComponent->velocity.y > 0.1f))
			{
				velocityComponent->velocity.y = 0.0f;
			}
		}
	});


	// ponowne umożliwienie skakania po dotknięciu podłoża
	// chleb

	eventSystem.registerListener<CollisionEvent>([&](const Event& e) {
		const auto& event = static_cast<const CollisionEvent&>(e);
		if (!event.isColliding) return;

		if (!scene->hasComponent<BreadController>(event.objectA))
			return;

		BreadController* breadController = &scene->getComponent<BreadController>(event.objectA);

		if (glm::length(event.separationVector) != 0.0f &&
			glm::dot(glm::normalize(event.separationVector), glm::vec3(0.0f, 1.0f, 0.0f)) > 0.9f)
		{
			breadController->isJumping = false;
			breadController->timeSinceLastGroundContact = 0.0f;
		}
	});

	// masło

	eventSystem.registerListener<CollisionEvent>([&](const Event& e) {
		const auto& event = static_cast<const CollisionEvent&>(e);
		if (!event.isColliding) return;

		if (!scene->hasComponent<ButterController>(event.objectA))
			return;

		ButterController* butterController = &scene->getComponent<ButterController>(event.objectA);

		if (glm::length(event.separationVector) != 0.0f &&
			glm::dot(glm::normalize(event.separationVector), glm::vec3(0.0f, 1.0f, 0.0f)) > 0.9f)
		{
			butterController->isJumping = false;
			butterController->timeSinceLastGroundContact = 0.0f;
			int floorProperties = scene->getComponent<ColliderComponent>(event.objectB).properties;
			butterController->canLeaveTrail = !(floorProperties & ColliderPropertyFlags::DisableButterTrail);
		}
	});


	// odbijanie się masła od chleba

	eventSystem.registerListener<CollisionEvent>([&](const Event& e) {
		const auto& event = static_cast<const CollisionEvent&>(e);
		if (!event.isColliding) return;

		if (!scene->hasComponent<ButterController>(event.objectA) ||
			!scene->hasComponent<VelocityComponent>(event.objectA) ||
			!scene->hasComponent<BreadController>(event.objectB))
			return;

		if (glm::length(event.separationVector) == 0.0f) return;

		ButterController& butter = scene->getComponent<ButterController>(event.objectA);
		BreadController& bread = scene->getComponent<BreadController>(event.objectB);

		if (bread.isBouncy)
		{
			auto& velocityComponent = scene->getComponent<VelocityComponent>(event.objectA);
			

			if (glm::dot(glm::normalize(event.separationVector), {0.0f, 1.0f, 0.0f}) > 0.9f
				&& velocityComponent.velocity.y < 0.1f)
			{
				scene->getAudioSystem().playSound(event.objectB);

				velocityComponent.velocity.y = butter.jumpSpeed * 1.5f;
				butter.isJumping = true;
			}
		}
	});
	//heat
	eventSystem.registerListener<CollisionEvent>([&](const Event& e)
		{
			const auto& ev = static_cast<const CollisionEvent&>(e);

			if (!scene->hasComponent<ButterController>(ev.objectA) ||
				!scene->hasComponent<ButterHealthComponent>(ev.objectA) ||
				!scene->hasComponent<HeatComponent>(ev.objectB))
				return;

			scene->getComponent<ButterHealthComponent>(ev.objectA).burning = true;

			auto& butter = scene->getComponent<ButterController>(ev.objectA);
			butter.inHeat = true;
			butter.trailBurstLeft = 5.0f;
		});


	//freeze
	eventSystem.registerListener<CollisionEvent>([&](const Event& e)
		{
			const auto& ev = static_cast<const CollisionEvent&>(e);
			if (!ev.isColliding) return;

			auto isFreeze = [&](EntityID id)
				{ return scene->hasComponent<FreezeComponent>(id); };

			auto isChleb = [&](EntityID id)
				{ return scene->hasComponent<ObjectInfoComponent>(id) &&
				scene->getComponent<ObjectInfoComponent>(id).tag == "chleb"; };

			bool aFreeze = isFreeze(ev.objectA);
			bool bFreeze = isFreeze(ev.objectB);
			bool aChleb = isChleb(ev.objectA);
			bool bChleb = isChleb(ev.objectB);

			if ((aFreeze && bChleb) || (bFreeze && aChleb))
			{
				EntityID breadID = aChleb ? ev.objectA : ev.objectB;
				scene->getComponent<BreadController>(breadID).freezing = true;
			}

		});



	//regen
	eventSystem.registerListener<CollisionEvent>([&](const Event& e)
		{
			const auto& ev = static_cast<const CollisionEvent&>(e);
			if (!ev.isColliding) return;

			auto isMaslo = [&](EntityID id) { return scene->hasComponent<ObjectInfoComponent>(id)
				&& scene->getComponent<ObjectInfoComponent>(id).tag == "maslo"; };
			auto isRegen = [&](EntityID id) { return scene->hasComponent<RegenComponent>(id); };

			bool condition = (isMaslo(ev.objectA) && isRegen(ev.objectB)) ||
				(isMaslo(ev.objectB) && isRegen(ev.objectA));
			if (!condition) return;


			auto& regen = scene->getComponent<RegenComponent>(
				isRegen(ev.objectA) ? ev.objectA : ev.objectB);
			spdlog::info("{}", regen.OnEnterMessage);


			auto& bh = scene->getComponent<ButterHealthComponent>(
				isMaslo(ev.objectA) ? ev.objectA : ev.objectB);
			bh.healing = true;
		});


	eventSystem.registerListener<CollisionEvent>([&](const Event& e) {
		const auto& ev = static_cast<const CollisionEvent&>(e);
		if (!ev.isColliding) return;

		if (!(scene->hasComponent<ButterController>(ev.objectA)
			|| scene->hasComponent<BreadController>(ev.objectA))
			|| !scene->hasComponent<ButtonComponent>(ev.objectB))
		{
			return;
		}


		//if (ev.separationVector.y < 0.01f) return;
		if (glm::length(ev.separationVector) == 0.0f ||
			glm::dot(glm::normalize(ev.separationVector), glm::vec3(0.0f, 1.0f, 0.0f)) < 0.9f)
		{
			return;
		}

		auto& button = scene->getComponent<ButtonComponent>(ev.objectB);
		if (button.elevatorEntity == (EntityID)-1) return;

		bool allow = true;
		if (button.activatorMode == ButtonComponent::ActivatorMode::AllByTag &&
			!button.activatorTag.empty())
		{
			auto& info = scene->getComponent<ObjectInfoComponent>(ev.objectA);
			allow = (info.tag == button.activatorTag);
		}
		if (!allow) return;

		auto& elevator = scene->getComponent<ElevatorComponent>(button.elevatorEntity);

		if (!elevator.isMoving) {
			auto& transform = scene->getComponent<Transform>(elevator.id);
			elevator.startY = transform.translation.y;
			elevator.isMoving = true;
		}
	});

	//slad masla sprint
	eventSystem.registerListener<TriggerEvent>([&](const Event& e) {
		const auto& ev = static_cast<const TriggerEvent&>(e);

		if (!(scene->getComponent<ObjectInfoComponent>(ev.triggerObject).tag == "trail") ||
			!scene->hasComponent<TrailCollisionDetectorComponent>(ev.otherObject))
			return;

		// odśwież licznik sprintu do 3 s
		scene->getComponent<TrailCollisionDetectorComponent>(ev.otherObject).sprintTimeLeft = 3.0f;
		});

	//maslo przyczepia sie do sciany
	eventSystem.registerListener<CollisionEvent>([&](const Event& e)
		{
			const auto& ev = static_cast<const CollisionEvent&>(e);
			if (!ev.isColliding) return;


			if (!scene->hasComponent<ButterController>(ev.objectA) ||
				!scene->hasComponent<ColliderComponent>(ev.objectB))
				return;

			auto& wallCollider = scene->getComponent<ColliderComponent>(ev.objectB);
			if (wallCollider.properties & ColliderPropertyFlags::DisableButterSticking) return;

			auto& butter = scene->getComponent<ButterController>(ev.objectA);
			if (!butter.isSticky || butter.isClinging)
				return;

			auto& velocity = scene->getComponent<VelocityComponent>(ev.objectA);

			if (glm::length(ev.separationVector) == 0.0f)
				return;
			glm::vec3 n = glm::normalize(ev.separationVector);
			if (std::abs(n.y) > 0.3f) return;

		
			glm::vec3 linearVel = velocity.velocity;
			linearVel.y = 0.0f;



			bool pushing = false;
			
			if (glm::length(linearVel) > 0.0f)
			{
				linearVel = glm::normalize(linearVel);
				pushing = glm::dot(linearVel, -n) > 0.5f;
			}

			if (pushing)
			{
				auto& collider = scene->getComponent<ColliderComponent>(butter.id);
				collider.isStatic = true;

				butter.isClinging = true;
				butter.clingNormal = n;

				EntityID clingEntity = scene->createEntity(ev.objectB);
				auto& ts = scene->getTransformSystem();
				ts.setGlobalMatrix(clingEntity, scene->getComponent<Transform>(ev.objectA).globalMatrix);

				glm::mat4 wallMatrix = scene->getComponent<Transform>(ev.objectB).globalMatrix;

				glm::vec3 upLocal = glm::inverse(wallMatrix) * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
				if (glm::length(upLocal) > 0.0f) upLocal = glm::normalize(upLocal);

				glm::quat rotation = glm::quatLookAt(-n, upLocal);
				ts.rotateEntity(clingEntity, rotation);

				butter.clingEntity = clingEntity;

				velocity.useGravity = false;
				velocity.velocity = glm::vec3(0.0f);  

				if (butter.clingColliderExtension != (EntityID)-1)
				{
					auto& extCol = scene->getComponent<ColliderComponent>(butter.clingColliderExtension);
					extCol.properties &= ~(ColliderPropertyFlags::DisableCollider);
				}
			}
		});


	// przechodzenie do następnego poziomu
	eventSystem.registerListener<TriggerEvent>([&](const Event& e) {
		const auto& ev = static_cast<const TriggerEvent&>(e);

		if (!scene->hasComponent<LevelExitComponent>(ev.triggerObject) ||
			(!scene->hasComponent<ButterController>(ev.otherObject) &&
				!scene->hasComponent<BreadController>(ev.otherObject)))
			return;

		auto& levelExit = scene->getComponent<LevelExitComponent>(ev.triggerObject);

		++levelExit.playerCount;
		if (levelExit.active && levelExit.playerCount == 2)
		{
			if (!levelExit.nextLevelPath.empty())
			{
				++currentLevel;
				changeSceneTo = "res/scenes/" + levelExit.nextLevelPath;
			}
			auto it = dialogues.find(levelExit.dialogueName);
			if (it != dialogues.end())
			{
				auto& dialogueVec = it->second;

				if (!currentDialogue.empty())
				{
					currentDialogue = std::queue<DialogueBox>();
				}

				for (auto& dialogueBox : dialogueVec)
				{
					currentDialogue.push(dialogueBox);
				}
				scene->playerLock = true;
			}

			levelExit.active = false;
		}
	});

	eventSystem.registerListener<TriggerEvent>([&](const Event& e) {
		const auto& ev = static_cast<const TriggerEvent&>(e);

		if (!scene->hasComponent<CollectibleComponent>(ev.triggerObject) ||
			(!scene->hasComponent<ButterController>(ev.otherObject) &&
				!scene->hasComponent<BreadController>(ev.otherObject)))
			return;

		auto& collectible = scene->getComponent<CollectibleComponent>(ev.triggerObject);

		auto& collider = scene->getComponent<ColliderComponent>(ev.triggerObject);
		collider.properties |= ColliderPropertyFlags::DisableCollider;

		for (EntityID child : scene->getComponent<Transform>(ev.triggerObject).children)
		{
			scene->destroyEntity(child);
		}
		scene->playerLock = true;
		EntityID bun = scene->instantiatePrefab("Bula")[0];
		scene->getComponent<BunController>(bun).joke = jokes[currentLevel % jokes.size()];
		scene->getAudioSystem().playSound("res/sounds/znajdzka.wav");
	});
}

void Application::setStartValues()
{
	scene->getRenderingSystem().getTexture("res/textures/dialogtlo.png");

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
	auto butterControllers = scene->getStorage<ButterController>();
	if (butterControllers)
	{
		for (int i = 0; i < butterControllers->getQuantity(); i++)
		{
			auto& butter = butterControllers->components[i];
			auto& transform = scene->getComponent<Transform>(butter.id);

			for (EntityID child : transform.children)
			{
				if (scene->getComponent<ObjectInfoComponent>(child).tag == "clingColliderExtension" &&
					scene->hasComponent<ColliderComponent>(child))
				{
					butter.clingColliderExtension = child;
					break;
				}
			}
		}
	}

	if (auto collectibles = scene->getStorage<CollectibleComponent>())
	{
		constexpr float collectibleOscTime = 4.0f;
		constexpr float collectibleOscAmplitude = 0.5f;

		for (int i = 0; i < collectibles->getQuantity(); ++i)
		{
			auto& collectible = collectibles->components[i];
			auto& transform = scene->getComponent<Transform>(collectible.id);

			collectible.startPosition = transform.translation;
		}

		int selectedCollectibleIndex = Random::getInt(0, collectibles->getQuantity());

		auto& selectedCollectible = collectibles->components[selectedCollectibleIndex];
		scene->instantiatePrefab("CollectibleModel", selectedCollectible.id);

		auto& selectedCollider = scene->getComponent<ColliderComponent>(selectedCollectible.id);
		selectedCollider.properties &= ~(ColliderPropertyFlags::DisableCollider);

	}


	//TODO WOLNE
	EntityID ost = scene->getEntityByName("ost");
	if(ost != (EntityID) -1) {
		scene->getAudioSystem().playSound(ost);
	}
}
