#pragma once
#include "glfw/glfw3.h"
#include "ECS/EntityManager.h"
#include <queue>

class Scene;


struct ButterController
{
	float moveSpeed;
	float jumpSpeed;
	bool isJumping = false;
	float timeSinceLastGroundContact = 0.0f;
    EntityID respawnPoint = (EntityID)-1;

	std::queue<EntityID> trailEntities;

	void update(GLFWwindow* window, Scene* scene, float deltaTime);


	EntityID id = (EntityID)-1;
};