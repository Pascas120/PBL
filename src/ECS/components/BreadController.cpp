#include "BreadController.h"
#include <glm/glm.hpp>
#include "Scene.h"

void BreadController::update(GLFWwindow* window, Scene* scene, float deltaTime)
{
	auto& transformSystem = scene->getTransformSystem();

	// Na tą chwilę rotacja przy ruchu jest ograniczona tylko do 4 stron świata, w przyszłości można to zmienić na bardziej płynne obracanie
	if (scene->hasComponent<VelocityComponent>(id))
	{
		auto& velocityComponent = scene->getComponent<VelocityComponent>(id);
		glm::vec3 movement(0.0f, 0.0f, 0.0f);
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		{
			movement.z -= moveSpeed;
			transformSystem.rotateEntity(id, glm::vec3(0.0f, 180.0f, 0.0f), deltaTime*10);
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		{
			movement.z += moveSpeed;
			transformSystem.rotateEntity(id, glm::vec3(0.0f, 0.0f, 0.0f), deltaTime*10);
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		{
			movement.x -= moveSpeed;
			transformSystem.rotateEntity(id, glm::vec3(0.0f, 270.0f, 0.0f), deltaTime*10);
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		{
			movement.x += moveSpeed;
			transformSystem.rotateEntity(id, glm::vec3(0.0f, 90.0f, 0.0f), deltaTime*10);
		}

		if (glm::length(movement) > 0.0f)
		{
			movement = glm::normalize(movement) * moveSpeed;
		}

		if (!isJumping && glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		{
			movement.y += jumpSpeed;
			isJumping = true;
		}
		else
		{
			movement.y = velocityComponent.velocity.y;
		}

		velocityComponent.velocity = movement;

	}


}