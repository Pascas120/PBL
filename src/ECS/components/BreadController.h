#pragma once
#include "GLFW/glfw3.h"
#include "ECS/EntityManager.h"
#include <glm/glm.hpp>
class Scene;

struct BreadController {
	float moveSpeed;
	float jumpSpeed;
	bool isJumping = false;
	float timeSinceLastGroundContact = 0.0f;

	glm::vec3 startScale = { 1.0f, 1.0f, 1.0f };
	float relativeScale = 1.0f;
	bool isBouncy = false;
	void update(GLFWwindow* window, Scene* scene, float deltaTime);
	EntityID id = (EntityID)-1;
};