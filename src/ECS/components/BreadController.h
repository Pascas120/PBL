#pragma once
#include "GLFW/glfw3.h"
#include "ECS/EntityManager.h"
#include <glm/glm.hpp>
class Scene;

struct BreadController {
	float moveSpeed;
	float jumpSpeed;
	EntityID respawnPoint = (EntityID)-1;
	bool isJumping = false;
	float timeSinceLastGroundContact = 0.0f;

	glm::vec3 targetRotation = glm::vec3(0.0f, 0.0f, 0.0f);

	bool  freezing = false;

	float freezeCooldownTimer = 0.0f;     
	static constexpr float freezeCooldownDur = 0.25f;

	glm::vec3 startScale = { 1.0f, 1.0f, 1.0f };
	float relativeScale = 1.0f;
	bool isBouncy = false;
	void update(GLFWwindow* window, Scene* scene, float deltaTime);
	EntityID id = (EntityID)-1;
};