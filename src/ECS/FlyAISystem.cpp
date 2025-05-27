#include "FlyAISystem.h"
#include "Scene.h"
#include "Components.h"
#include <glm/gtc/quaternion.hpp>
#include <random>
#include <cmath>

FlyAISystem::FlyAISystem(Scene* scene) : scene(scene) {}

void FlyAISystem::update() {
	auto transforms = scene->getStorage<Transform>();
	auto flyAIComponents = scene->getStorage<FlyAIComponent>();
	for (int i = 0; i < flyAIComponents->getQuantity(); i++) {
		auto& flyAI = flyAIComponents->components[i];
		auto& transform = transforms->get(flyAI.id);
		FlyAIAndTransform flyComp{ flyAI, transform };
		if (flyAI.idButter == (EntityID)-1) continue;
        if (flyAI.diveCooldownTimer > 0.0f)
            flyAI.diveCooldownTimer -= deltaTime;

		float butterDistance = glm::distance(transform.globalMatrix[3], transforms->get(flyAI.idButter).globalMatrix[3]);
        float patrolHeight = flyAI.groundY + flyAI.patrolHeightOffset;

        switch (flyAI.state)
        {
        case FlyAIComponent::Patrolling:
            patrol(flyComp, patrolHeight);
            if (butterDistance < flyAI.detectionRadius && flyAI.diveCooldownTimer <= 0.0f)
                flyAI.state = FlyAIComponent::Diving;
            break;

        case FlyAIComponent::Diving:
            dive(flyComp);
            if (butterDistance > flyAI.detectionRadius || transform.globalMatrix[3].y <= flyAI.diveEndHeight)
                flyAI.state = FlyAIComponent::Returning;
            break;

        case FlyAIComponent::Returning:
            returnToPatrolHeight(flyComp, patrolHeight);
            if (abs(transform.globalMatrix[3].y - patrolHeight) < 0.1f)
            {
                flyAI.state = FlyAIComponent::Patrolling;
                chooseNewPatrolPoint(flyComp);
            }
            break;
        }
	}
}

void FlyAISystem::patrol(const FlyAIAndTransform& flyComp, float patrolHeight) {
    auto& transform = flyComp.transform;
    auto& flyAI = flyComp.flyAI;
    glm::vec3 direction = flyAI.patrolTarget - glm::vec3(transform.globalMatrix[3]);
    direction.y = 0.f;
	direction = glm::normalize(direction);

    glm::vec3 position = transform.globalMatrix[3];
    position.y = patrolHeight;
    

    lookAt2D(flyComp, flyAI.patrolTarget);
    if (glm::distance(position, flyAI.patrolTarget) < flyAI.patrolPointReachedThreshold)
    {
        chooseNewPatrolPoint(flyComp);
    }
}

void FlyAISystem::lookAt2D(const FlyAIAndTransform& flyComp, glm::vec3 target)
{
    auto& transform = flyComp.transform;
    auto& flyAI = flyComp.flyAI;
    auto& ts = scene->getTransformSystem();
	glm::vec3 dir = target - glm::vec3(transform.globalMatrix[3]);
    dir.y = 0;
    if (dir != glm::vec3(0))
    {
        glm::quat rot = glm::quatLookAt(dir, glm::vec3(0,1,0));
        ts.rotateEntity(flyAI.id, glm::slerp(transform.rotation, rot, deltaTime * 5.f));
    }
}

static glm::vec2 randomInUnitCircle() {
    static std::mt19937 rng(std::random_device{}());
    static std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    float angle = dist(rng) * 2.0f * glm::pi<float>();
    float radius = std::sqrt(dist(rng)); // sqrt for uniform distribution
    return glm::vec2(std::cos(angle), std::sin(angle)) * radius;
}

void FlyAISystem::chooseNewPatrolPoint(const FlyAIAndTransform& flyComp)
{
    auto& transform = flyComp.transform;
    auto& flyAI = flyComp.flyAI;
    glm::vec2 offset = randomInUnitCircle() * flyAI.patrolRange;
	flyAI.patrolTarget = glm::vec3(transform.globalMatrix[3]) + glm::vec3(offset.x, 0.0f, offset.y);
}

void FlyAISystem::dive(const FlyAIAndTransform& flyComp)
{
    auto& transform = flyComp.transform;
    auto& flyAI = flyComp.flyAI;
	auto& butter = scene->getStorage<Transform>()->get(flyAI.idButter);
    auto& ts = scene->getTransformSystem();
	glm::vec4 direction = butter.globalMatrix[3] - transform.globalMatrix[3];
    direction.w = 0.0f;
    direction = glm::normalize(direction);
    transform.globalMatrix[3] += direction * flyAI.diveSpeed * deltaTime;
    ts.setGlobalMatrix(flyAI.id, transform.globalMatrix);
    lookAt2D(flyComp, butter.globalMatrix[3]);
}

void FlyAISystem::returnToPatrolHeight(const FlyAIAndTransform& flyComp, float patrolHeight)
{
    auto& transform = flyComp.transform;
    auto& flyAI = flyComp.flyAI;
    auto& ts = scene->getTransformSystem();
	glm::vec3 target = transform.globalMatrix[3];
	target.y = patrolHeight;
	glm::vec3 direction = glm::vec3(target - glm::vec3(transform.globalMatrix[3]));
    direction = glm::normalize(direction);
	glm::vec3 position = transform.globalMatrix[3];
	position += direction * flyAI.returnSpeed * deltaTime;
	transform.globalMatrix[3] = glm::vec4(position, 0.0f);
    ts.setGlobalMatrix(flyAI.id, transform.globalMatrix);
    lookAt2D(flyComp, flyAI.patrolTarget);
}