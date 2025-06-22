#pragma once
#include "glfw/glfw3.h"
#include "ECS/EntityManager.h"
#include <queue>
#include "glm/glm.hpp"

class Scene;


struct ButterController
{

	float moveSpeed;
	float jumpSpeed;
	bool isJumping = false;
	float timeSinceLastGroundContact = 0.0f;
    EntityID respawnPoint = (EntityID)-1;

	glm::vec3 targetRotation = glm::vec3(0.0f, 0.0f, 0.0f);

	std::queue<EntityID> trailEntities;

	bool  inHeat = false;  
	//bool  wasInHeat = false;   
	float trailBurstLeft = 0.f;   
	//float trailCooldown = 0.f;    

	bool floating = false;


	void update(GLFWwindow* window, Scene* scene, float deltaTime);

	void addTrailIfPossible(Scene* scene);

	EntityID id = (EntityID)-1;	
	bool isSticky = false;
	bool isClinging = false;
	glm::vec3 clingNormal = {};
	EntityID clingEntity = (EntityID)-1;
};