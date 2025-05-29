#include "BreadController.h"
#include <glm/glm.hpp>
#include "Scene.h"

void BreadController::update(GLFWwindow* window, Scene* scene, float deltaTime) {
	if (scene->hasComponent<VelocityComponent>(id)) 
	{
		auto& velocity = scene->getComponent<VelocityComponent>(id);
		glm::vec3 movement(0.0f, 0.0f, 0.0f);
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
			movement.z += moveSpeed;
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
			movement.z -= moveSpeed;
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
			movement.x += moveSpeed;
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
			movement.x -= moveSpeed;
		}
		if (glm::length(movement) > 0.0f) {
			movement = glm::normalize(movement) * moveSpeed;
		}

		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !isJumping) {
			movement.y += jumpSpeed;
			isJumping = true;
		}
		else
		{
			movement.y -= velocity.velocity.y;
		}

		velocity.velocity = movement;
	}
}