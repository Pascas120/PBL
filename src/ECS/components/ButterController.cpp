#include "ButterController.h"

#include "glm/glm.hpp"
#include "Scene.h"
#include "spdlog/spdlog.h"

void ButterController::update(GLFWwindow* window, Scene* scene, float deltaTime)
{
	if (scene->hasComponent<VelocityComponent>(id))
	{
		auto& velocityComponent = scene->getComponent<VelocityComponent>(id);
		glm::vec3 movement(0.0f, 0.0f, 0.0f);
		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		{
			movement.z += moveSpeed;
		}
		if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		{
			movement.z -= moveSpeed;
		}
		if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		{
			movement.x += moveSpeed;
		}
		if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		{
			movement.x -= moveSpeed;
		}

		if (glm::length(movement) > 0.0f)
		{
			movement = glm::normalize(movement) * moveSpeed;
		}

		if (!isJumping && glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS)
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