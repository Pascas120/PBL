#pragma once

#include "glfw/glfw3.h"
#include "ECS/EntityManager.h"
#include "glm/glm.hpp"

class Scene;

struct SplitScreenController
{
    EntityID target1;
	EntityID camera1;


	EntityID target2;
	EntityID camera2;

	bool splitActive = false;
	float splitSlope = 0.0f;
	bool target1AboveSlope;
	float splitLineThickness = 0.0f;


    glm::vec3 offset = { 0.0f, 2.0f, 5.0f };

    void update(GLFWwindow* window, Scene* scene, float deltaTime);


    EntityID id = (EntityID)-1;
};