#include "BreadController.h"
#include <glm/glm.hpp>
#include "Scene.h"

void BreadController::update(GLFWwindow* window, Scene* scene, float deltaTime)
{
	auto& transformSystem = scene->getTransformSystem();


	
	float rate = deltaTime / freezeDuration;     

	if (freezing)         
		freezeRatio = glm::clamp(freezeRatio + rate, 0.0f, 1.0f);
	else                  
		freezeRatio = glm::clamp(freezeRatio - rate, 0.0f, 1.0f);

	float freezeFactor = 1.0f - freezeRatio;     
	
	float rotMul = freezeFactor;

	// Na tą chwilę rotacja przy ruchu jest ograniczona tylko do 4 stron świata, w przyszłości można to zmienić na bardziej płynne obracanie
	if (scene->hasComponent<VelocityComponent>(id))
	{
		auto& velocityComponent = scene->getComponent<VelocityComponent>(id);
		glm::vec3 movement(0.0f, 0.0f, 0.0f);
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		{
			movement.z -= moveSpeed * freezeFactor;
			transformSystem.rotateEntity(id, glm::vec3(0.0f, 0.0f, 0.0f), deltaTime*10 * rotMul);
		}
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		{
			movement.z += moveSpeed * freezeFactor;
			transformSystem.rotateEntity(id, glm::vec3(0.0f, 180.0f, 0.0f), deltaTime*10 * rotMul);
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		{
			movement.x -= moveSpeed * freezeFactor;
			transformSystem.rotateEntity(id, -glm::vec3(0.0f, 270.0f, 0.0f), deltaTime*10 * rotMul);
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		{
			movement.x += moveSpeed * freezeFactor;
			transformSystem.rotateEntity(id, -glm::vec3(0.0f, 90.0f, 0.0f), deltaTime*10 * rotMul);
		}

		if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
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
				isBouncy = false;
			}
		}

		if (glm::length(movement) > 0.0f)
		{
			movement = glm::normalize(movement) * moveSpeed*freezeFactor;
		}

		timeSinceLastGroundContact += deltaTime;
		if (!isJumping && timeSinceLastGroundContact > 0.3f)
		{
			isJumping = true;
		}

		if (!isJumping && glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		{
			movement.y += jumpSpeed * freezeFactor;
			isJumping = true;
		}
		else
		{
			movement.y = velocityComponent.velocity.y;
		}

		velocityComponent.velocity = movement;

	}

	freezing = false;
}