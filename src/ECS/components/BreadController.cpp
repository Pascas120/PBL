#include "BreadController.h"
#include <glm/glm.hpp>
#include "Scene.h"
#include "spdlog/spdlog.h"
#include <glm/gtx/quaternion.hpp>

void BreadController::update(GLFWwindow* window, Scene* scene, float deltaTime)
{
	auto& transformSystem = scene->getTransformSystem();
	if (freezeCooldownTimer > 0.0f)
		freezeCooldownTimer -= deltaTime;

	float moveSpeed = this->moveSpeed;

	transformSystem.rotateEntity(id, targetRotation,
		deltaTime * 15.0f);

	if (freezing)
	{
		freezeCooldownTimer = freezeCooldownDur;         

		if (isBouncy)                                 
		{

			isBouncy = false;
		}
	}

	// Na tą chwilę rotacja przy ruchu jest ograniczona tylko do 4 stron świata, w przyszłości można to zmienić na bardziej płynne obracanie
	if (scene->hasComponent<VelocityComponent>(id))
	{
		GLFWgamepadstate state;
		glfwGetGamepadState(GLFW_JOYSTICK_2, &state);

		auto& velocityComponent = scene->getComponent<VelocityComponent>(id);
		glm::vec3 movement(0.0f, 0.0f, 0.0f);

		glm::vec2 inputAxes = { state.axes[GLFW_GAMEPAD_AXIS_LEFT_X], state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y] };
		movement.x = -inputAxes.x;
		movement.z = -inputAxes.y;

		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		{
			movement.z -= 1.0f;
		}
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		{
			movement.z += 1.0f;
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		{
			movement.x -= 1.0f;
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
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
		}

		movement *= moveSpeed;

		bool inflateButton = glfwGetKey(window, GLFW_KEY_F) ||
			state.buttons[GLFW_GAMEPAD_BUTTON_LEFT_BUMPER] ||
			state.buttons[GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER] ||
			state.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER] > 0.5f ||
			state.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER] > 0.5f;

		if (freezeCooldownTimer <= 0.0f && inflateButton)
		{
			if (relativeScale < 1.5f)
			{
				relativeScale += deltaTime * 0.8f;
				relativeScale = glm::clamp(relativeScale, 1.0f, 1.25f);
				transformSystem.scaleEntity(id, startScale * relativeScale);
				isBouncy = true;
			}
		}
		else       
		{
			if (relativeScale > 1.0f)
			{
				relativeScale -= deltaTime * 0.8f;
				relativeScale = glm::clamp(relativeScale, 1.0f, 1.25f);
				transformSystem.scaleEntity(id, startScale * relativeScale);
			}
			isBouncy = false;
		}

		timeSinceLastGroundContact += deltaTime;
		if (!isJumping && timeSinceLastGroundContact > 0.15f)
		{
			isJumping = true;
		}

		bool jumpButton = glfwGetKey(window, GLFW_KEY_SPACE) ||
			state.buttons[GLFW_GAMEPAD_BUTTON_A] ||
			state.buttons[GLFW_GAMEPAD_BUTTON_B] ||
			state.buttons[GLFW_GAMEPAD_BUTTON_X] ||
			state.buttons[GLFW_GAMEPAD_BUTTON_Y];

		if (!isJumping && jumpButton)
		{
			
			float jumpMul = freezing ? 0.5f : 1.0f;
			movement.y += jumpSpeed * jumpMul;
			isJumping = true;
		}
		else
		{
			movement.y = velocityComponent.velocity.y;
		}

		velocityComponent.velocity = movement;

	}

	auto& transform = scene->getComponent<Transform>(id);

	if(transform.translation.y<-9.5){
		transformSystem.translateEntity(id, scene->getComponent<Transform>(respawnPoint).translation);
	}

	freezing = false;
}