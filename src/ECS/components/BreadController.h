#pragma once
#include "GLFW/glfw3.h"
#include "ECS/EntityManager.h"
class Scene;

struct BreadController {
	float moveSpeed;
	float jumpSpeed;
	bool isJumping = false;
	void update(GLFWwindow* window, Scene* scene, float deltaTime);
	EntityID id = (EntityID)-1;
};