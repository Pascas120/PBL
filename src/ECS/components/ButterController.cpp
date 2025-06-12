#include "ButterController.h"

#include "glm/glm.hpp"
#include "Scene.h"
#include "spdlog/spdlog.h"

void ButterController::update(GLFWwindow* window, Scene* scene, float deltaTime)
{
	auto& transformSystem = scene->getTransformSystem();
	if (trailBurstLeft > 0.f)
	{
		const float SPAWN_EVERY = 0.05f;     
		trailBurstLeft -= deltaTime;
		trailCooldown -= deltaTime;

		if (trailCooldown <= 0.f)
		{
			addTrailIfPossible(scene);
			trailCooldown = SPAWN_EVERY;
		}
	}
	// Na tą chwilę rotacja przy ruchu jest ograniczona tylko do 4 stron świata, w przyszłości można to zmienić na bardziej płynne obracanie
	if (scene->hasComponent<VelocityComponent>(id))
	{
		auto& velocityComponent = scene->getComponent<VelocityComponent>(id);
        auto& transform = scene->getComponent<Transform>(id);
		glm::vec3 movement(0.0f, 0.0f, 0.0f);
		if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		{
			movement.z -= moveSpeed;
			transformSystem.rotateEntity(id, glm::vec3(0.0f, 0.0f, 0.0f), deltaTime*10);
		}
		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		{
			movement.z += moveSpeed;
			transformSystem.rotateEntity(id, glm::vec3(0.0f, 180.0f, 0.0f), deltaTime*10);
		}
		if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		{
			movement.x -= moveSpeed;
			transformSystem.rotateEntity(id, -glm::vec3(0.0f, 270.0f, 0.0f), deltaTime*10);
		}
		if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		{
			movement.x += moveSpeed;
			transformSystem.rotateEntity(id, -glm::vec3(0.0f, 90.0f, 0.0f), deltaTime*10);
		}

		if (glm::length(movement) > 0.0f)
		{
			movement = glm::normalize(movement) * moveSpeed;
		}

		timeSinceLastGroundContact += deltaTime;
		if (!isJumping && timeSinceLastGroundContact > 0.3f)
		{
			isJumping = true;
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

		if (!floating && glfwGetKey(window, GLFW_KEY_SLASH) == GLFW_PRESS)
		{
			velocityComponent.useGravity = false;
			if (scene->hasComponent<ColliderComponent>(id))
			{
				auto& collider = scene->getComponent<ColliderComponent>(id);
				collider.isStatic = true;
			}
			floating = true;
		}
		else if (floating && glfwGetKey(window, GLFW_KEY_SLASH) == GLFW_RELEASE)
		{
			velocityComponent.useGravity = true;
			if (scene->hasComponent<ColliderComponent>(id))
			{
				auto& collider = scene->getComponent<ColliderComponent>(id);
				collider.isStatic = false;
			}
			floating = false;
		}
		if (floating)
		{
			movement = glm::vec3(0.0f, 0.0f, 0.0f);
		}

		velocityComponent.velocity = movement;

        if(transform.translation.y<-9.5){
            transformSystem.translateEntity(id, scene->getComponent<Transform>(respawnPoint).translation);
        }


		
		if (wasInHeat && !inHeat)               
		{
			trailBurstLeft = 5.0f;               //czas5s
			trailCooldown = 0.f;              
		}
		wasInHeat = inHeat;                    
		inHeat = false;                       
		

	}



}
	void ButterController::addTrailIfPossible(Scene * scene)
	{
		auto& transformSystem = scene->getTransformSystem();
		auto& transform = scene->getComponent<Transform>(id);

	
		bool addTrail = (timeSinceLastGroundContact <= 0.1f);

		
		if (addTrail && !trailEntities.empty())
		{
			EntityID lastTrail = trailEntities.back();
			auto& lastTransform = scene->getComponent<Transform>(lastTrail);
			if (glm::length(lastTransform.translation - transform.translation) < 0.3f)
				addTrail = false;
		}

		if (!addTrail) return;

		
		EntityID trail = scene->instantiatePrefab("Trail")[0];

		float offsetScale = 1.0f;
		if (scene->hasComponent<ButterHealthComponent>(id))
		{
			auto& bh = scene->getComponent<ButterHealthComponent>(id);
			float lostRatio = 1.0f - (bh.timeLeft / bh.secondsToDie);
			offsetScale = glm::mix(1.0f, bh.minScale, lostRatio);
		}

		transformSystem.translateEntity(trail,
			transform.translation - glm::vec3(0.0f, 0.22f * offsetScale, 0.0f));
		transformSystem.rotateEntity(trail, transform.rotation);

		
		trailEntities.push(trail);
		if (trailEntities.size() > 200)
		{
			EntityID oldTrail = trailEntities.front();
			trailEntities.pop();
			scene->destroyEntity(oldTrail);
		}
	}