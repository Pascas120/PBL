#pragma once
#include <glm/glm.hpp>
class Scene;
struct FlyAIComponent;
struct Transform;
class FlyAISystem {
private:
	struct FlyAIAndTransform {
		FlyAIComponent& flyAI;
		Transform& transform;
	};
	Scene* scene;
	void patrol(const FlyAIAndTransform& flyComp, float patrolHeight);
	void chooseNewPatrolPoint(const FlyAIAndTransform& flyComp);
	void dive(const FlyAIAndTransform& flyComp);
	void returnToPatrolHeight(const FlyAIAndTransform& flyComp, float patrolHeight);
	void lookAt2D(const FlyAIAndTransform& flyComp, glm::vec3 target);
public:
	void update();
	FlyAISystem(Scene* scene);
	float deltaTime;
};