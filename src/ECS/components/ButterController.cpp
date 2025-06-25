#include "ButterController.h"

#include "glm/glm.hpp"
#include "Scene.h"
#include "spdlog/spdlog.h"
#include "Random.h"

void ButterController::update(GLFWwindow* window, Scene* scene, float deltaTime)
{
	auto& transformSystem = scene->getTransformSystem();
	auto& transform = scene->getComponent<Transform>(id);

	if (trailBurstLeft > 0.f && !inHeat)
	{
		const float SPAWN_EVERY = 0.05f;     
		trailBurstLeft -= deltaTime;
		//trailCooldown -= deltaTime;


		addTrailIfPossible(scene);
		/*if (trailCooldown <= 0.f)
		{
			trailCooldown = SPAWN_EVERY;
		}*/

	}
	GLFWgamepadstate state;
	glfwGetGamepadState(GLFW_JOYSTICK_1, &state);

	bool stickButton = glfwGetKey(window, GLFW_KEY_SLASH) ||
		state.buttons[GLFW_GAMEPAD_BUTTON_LEFT_BUMPER] ||
		state.buttons[GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER] ||
		state.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER] > 0.5f ||
		state.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER] > 0.5f;

	isSticky = stickButton;

	if (isClinging)
	{

		if (isSticky && scene->hasEntity(clingEntity))
		{
			auto& vel = scene->getComponent<VelocityComponent>(id);
			vel.useGravity = false;
			vel.velocity = glm::vec3(0.0f);

			glm::mat4 clingEntityMatrix = scene->getComponent<Transform>(clingEntity).globalMatrix;
			glm::vec3 clingEntPos = clingEntityMatrix[3];
			transformSystem.translateEntity(id, clingEntPos);

			glm::vec3 clingEntZ = glm::normalize(clingEntityMatrix[2]);
			glm::vec3 clingEntY = glm::normalize(clingEntityMatrix[1]);

			glm::vec3 normalEulerY = glm::eulerAngles(glm::quatLookAt(clingEntZ, clingEntY));
			normalEulerY = glm::degrees(normalEulerY);
			normalEulerY.y += 180.0f;
			transformSystem.rotateEntity(id, normalEulerY);

			return;                      
		}
		else
		{
			isClinging = false;

			if (scene->hasEntity(clingEntity))
			{
				scene->destroyEntity(clingEntity);
			}

			clingEntity = (EntityID)-1;
			auto& vel = scene->getComponent<VelocityComponent>(id);
			vel.useGravity = true;       

			auto& collider = scene->getComponent<ColliderComponent>(id);
			collider.isStatic = false;

			if (clingColliderExtension != (EntityID)-1)
			{
				auto& extCol = scene->getComponent<ColliderComponent>(clingColliderExtension);
				extCol.properties |= ColliderPropertyFlags::DisableCollider;
			}
		}
	}

	transformSystem.rotateEntity(id, targetRotation, deltaTime * 15.0f);
	// Na tą chwilę rotacja przy ruchu jest ograniczona tylko do 4 stron świata, w przyszłości można to zmienić na bardziej płynne obracanie
	if (scene->hasComponent<VelocityComponent>(id))
	{
		auto& velocityComponent = scene->getComponent<VelocityComponent>(id);
		glm::vec3 movement(0.0f, 0.0f, 0.0f);
		
		glm::vec2 inputAxes = { state.axes[GLFW_GAMEPAD_AXIS_LEFT_X], state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y] };
		movement.x = -inputAxes.x;
		movement.z = -inputAxes.y;

		if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		{
			movement.z -= 1.0f;
		}
		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		{
			movement.z += 1.0f;
		}
		if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		{
			movement.x -= 1.0f;
		}
		if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		{
			movement.x += 1.0f;
		}

		float movementMag = glm::length(movement);
		if (movementMag > 1e-4f)
		{
			glm::vec3 movementDir = movement / movementMag;
			float remappedMag = std::min(movementMag / 0.9f, 1.0f);
			movement = movementDir * remappedMag;
			
			targetRotation = glm::eulerAngles(glm::quatLookAt(movementDir, glm::vec3(0.0f, 1.0f, 0.0f)));
			targetRotation = glm::degrees(targetRotation);

			if(scene->hasComponent<AnimationComponent>(id)) {
				auto& anim = scene->getComponent<AnimationComponent>(id);
				if (movementMag > 0.1f && !anim.isPlaying)
				{
					scene->getAnimationSystem().playAnimation(id, "res/anims/maselkochodzenie.fbx");
				}
			}
		}

		movement *= moveSpeed;

		timeSinceLastGroundContact += deltaTime;
		if (!isJumping && timeSinceLastGroundContact > 0.15f)
		{
			isJumping = true;
		}

		bool jumpButton = glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) ||
			state.buttons[GLFW_GAMEPAD_BUTTON_A] ||
			state.buttons[GLFW_GAMEPAD_BUTTON_B] ||
			state.buttons[GLFW_GAMEPAD_BUTTON_X] ||
			state.buttons[GLFW_GAMEPAD_BUTTON_Y];

		if (!isJumping && jumpButton)
		{
			movement.y += jumpSpeed;
			isJumping = true;
			scene->getAudioSystem().playSound("res/sounds/skokMaslo.mp3");
		}
		else
		{
			movement.y = velocityComponent.velocity.y;
		}


		velocityComponent.velocity = movement;

        if(transform.translation.y<-9.5){
            transformSystem.translateEntity(id, scene->getComponent<Transform>(respawnPoint).translation);
        }
                  
		inHeat = false;
		

	}



}



void ButterController::addTrailIfPossible(Scene * scene)
{
	auto& transformSystem = scene->getTransformSystem();
	auto& transform = scene->getComponent<Transform>(id);

	
	if (!canLeaveTrail || timeSinceLastGroundContact != 0.0f) return;

	float offsetScale = 1.0f;
	if (scene->hasComponent<ButterHealthComponent>(id))
	{
		auto& bh = scene->getComponent<ButterHealthComponent>(id);
		float lostRatio = 1.0f - (bh.timeLeft / bh.secondsToDie);
		offsetScale = glm::mix(1.0f, bh.minScale, lostRatio);
	}

	glm::vec3 newTrailPos;
	bool defaultTrailPos = true;

	constexpr float minTrailDist = 0.4f;
		
	if (!trailEntities.empty())
	{
		EntityID lastTrail = trailEntities.back();
		auto& lastTransform = scene->getComponent<Transform>(lastTrail);
		glm::vec3 diff = transform.translation - lastTransform.translation;
		diff.y = 0.0f;

		float trailDist = glm::length(diff);
		float scaledMinTrailDist = minTrailDist * offsetScale;
		if (trailDist < scaledMinTrailDist) {
			return;
		}

		if (trailDist < 2 * scaledMinTrailDist)
		{
			newTrailPos = lastTransform.translation + glm::normalize(diff) * scaledMinTrailDist;
			defaultTrailPos = false;
		}
	}

	if (defaultTrailPos)
		newTrailPos = transform.translation - glm::vec3(0.0f, 0.22f * offsetScale, 0.0f);


	constexpr std::array trailNames = { "Trail1", "Trail2", "Trail3" };
	const std::string& trailName = trailNames[Random::getInt(0, trailNames.size() - 1)];
		
	auto trails = scene->instantiatePrefab(trailName);
	if (trails.empty()) return;
	EntityID trail = trails[0];


		
	transformSystem.translateEntity(trail, newTrailPos);

	float trailRotY = Random::getFloat(-180.0f, 180.0f);
	transformSystem.rotateEntity(trail, glm::vec3(0.0f, trailRotY, 0.0f));

	glm::vec3 newScale = transform.scale * 1.5f;
	newScale.y = transform.scale.y;
	transformSystem.scaleEntity(trail, newScale);

		
	trailEntities.push(trail);
	if (trailEntities.size() > 200)
	{
		EntityID oldTrail = trailEntities.front();
		trailEntities.pop();
		scene->destroyEntity(oldTrail);
	}



}