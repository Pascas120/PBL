//
// Created by Łukasz Moskwin on 14/04/2025.
//

#ifndef PBL_COMPONENTS_H
#define PBL_COMPONENTS_H
#include <array>

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include <string>
#include "Model.h"
#include "EntityManager.h"
#include <memory>

#include "Animator.h"
#include "ECS/BoundingVolumes.h"
#include "Camera.h"
#include "components/ColliderComponent.h"
#include "components/BreadController.h"
#include "components/CameraComponent.h"
#include "components/SplitScreenController.h"
#include "components/ButterController.h"
#include "miniaudio.h"

enum class FlyVariant : uint8_t {
    GREEN = 0,
    RED = 1,
    GOLD = 2,
    PURPLE = 3,
    COUNT
};
enum class ElevatorState { Closed, Opening, Open, Closing };
enum class ButtonLinkMode : uint8_t { Single = 0, DoubleD = 1 };
class Model;

struct ObjectInfoComponent {
    std::string name;
    std::string uuid;
    std::string tag;

	EntityID id;
};

struct Transform {
    bool isStatic = true;

    glm::vec3 translation = {0.0f, 0.0f, 0.0f};
    glm::quat rotation = {1.0f, 0.0f, 0.0f, 0.0f};
    glm::vec3 eulerRotation = glm::vec3(0.0f);
    glm::vec3 scale = {1.0f, 1.0f, 1.0f};
    glm::mat4 globalMatrix = glm::mat4(1.0f);

    bool isDirty = true;
    std::vector<EntityID> children;
    EntityID parent = (EntityID) -1;

    std::string uuid;

	EntityID id = (EntityID)-1;
};

struct ModelComponent {
    Shader* shader;
    Model* model;
    Transform* transform;
    glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);

	glm::mat4 prevModelMatrix = glm::mat4(1.0f);

    EntityID id = (EntityID)-1;
};

struct ImageComponent {
    std::string texturePath;
    float width;
    float height;
    glm::vec4 color;

    EntityID id = (EntityID)-1;
};

struct TextComponent {
    std::string font;
    glm::vec4 color;
    std::string text;
    float fontSize = 1.0f;

    EntityID id = (EntityID)-1;
};



struct PointLightComponent {
    glm::vec3 color = { 1.0f, 1.0f, 1.0f };
    float intensity = 1.0f;

    float constant = 1.0f;
    float linear = 0.09f;
    float quadratic = 0.032f;

    EntityID id = (EntityID)-1;
};

struct DirectionalLightComponent {
    glm::vec3 color = { 1.0f, 1.0f, 1.0f };
    float intensity = 1.0f;

    EntityID id = (EntityID)-1;
};

struct FlyAIComponent {
	EntityID idButter = (EntityID)-1;
    EntityID idBread = (EntityID)-1;
    float patrolHeightOffset = 3.f;
    float patrolSpeed = 3.f;
    float diveSpeed = 6.f;
    float detectionRadius = 8.f;
    float diveEndHeight = 0.4f;
    float returnSpeed = 3.5f;
    float patrolRange = 10.f;
    float patrolPointReachedThreshold = 0.5f;
    float diveCooldownTime = 2.f;
    float diveCooldownTimer = 0.f;
	float groundY = 0.f;
    enum FlyState { Patrolling, Diving, Returning }
    state = FlyState::Patrolling;
    glm::vec3 patrolTarget;
    enum PatrolAxis { Horizontal, Vertical } 
    patrolAxis = PatrolAxis::Horizontal;
	glm::vec3 patrolStart = { 0.0f, 0.0f, 0.0f };
	glm::vec3 patrolEnd = { 0.0f, 0.0f, 0.0f };
    bool movingForward = true;
    EntityID id = (EntityID)-1;
};

struct VelocityComponent {
	glm::vec3 velocity = { 0.0f, 0.0f, 0.0f };
	glm::vec3 angularVelocity = { 0.0f, 0.0f, 0.0f };
	bool useGravity = true;

	EntityID id = (EntityID)-1;
};

struct HeatComponent {
    float triggerRadius = 1.0f;
    bool  hasTriggered = false;        
    std::string OnEnterMessage = "cieplo";
    EntityID id = static_cast<EntityID>(-1);
};
struct RegenComponent {
    float triggerRadius = 1.0f;
    bool  hasTriggered = false;
    std::string OnEnterMessage = "regen";
    EntityID id = static_cast<EntityID>(-1);
};
struct FreezeComponent {
    float triggerRadius = 1.0f;
    bool hasTriggered = false;
    std::string OnEnterMessage = "zimno";
    EntityID id = static_cast<EntityID>(-1);
};
struct ButterHealthComponent
{
    float secondsToDie = 5.0f;
    float secondsToHeal = 5.0f;      
    float minScale = 0.5f;
    float timeLeft = secondsToDie;
    bool  burning = false;
    bool  healing = false;     
    glm::vec3 startScale;
    EntityID id = static_cast<EntityID>(-1);
};
struct ElevatorComponent {
    EntityID id;
    float openHeight;
    float speed;
    glm::vec3 closedPos = glm::vec3{ 0 };
    ElevatorState state = ElevatorState::Closed;
    EntityID buttonEntity;
    float maxHeight = 5.0f;
    bool isMoving = false;
    bool shouldOpen = false;
    bool hasInitClosedPos = false;
    float startY = -1.0f; 

    bool isDoor = false;
    enum class DoorDir { Left, Right };
    DoorDir doorDir = DoorDir::Left;      
    bool locked = false;
     

    bool  rotate = false;                 
    float rotateAngle = 90.f;             

    enum class RotateAxis { X = 0, Y = 1, Z = 2 };
    RotateAxis rotateAxis = RotateAxis::Y; 

    float startAngle = 0.f;
};

struct ButtonComponent {
    EntityID id;  
    enum class ActivatorMode : uint8_t { PlayersOnly = 0, AllByTag = 1 };
    ActivatorMode activatorMode = ActivatorMode::PlayersOnly;
    bool isPressed = false;
    float pressDepth = 0.1f;
    float pressSpeed = 4.0f;
    std::string activatorTag;
    EntityID elevatorEntity = static_cast<EntityID>(-1);

    ButtonLinkMode linkMode = ButtonLinkMode::Single;
    EntityID       linkedButton = static_cast<EntityID>(-1);
};

struct SoundComponent {
    std::string soundPath;
    ma_sound sound;
    bool loop = false;
    float volume = 1.0f;
    float pitch = 1.0f;
    bool isInitialized = false;

    EntityID id = (EntityID)-1;
};

struct AnimationComponent {
    Animator* animator = new Animator(nullptr);
    bool isPlaying = false;
    float playbackSpeed = 1.0f;
    bool loop = false;
    std::vector<Animation*> animations;

    EntityID id = (EntityID)-1;
};

struct TrailCollisionDetectorComponent
{
    float sprintTimeLeft = 0.0f;   
    float sprintMultiplier = 2.0f;   
    EntityID id = static_cast<EntityID>(-1);
};

struct LevelExitComponent
{
    std::string nextLevelPath;
    std::string dialogueName;
    int playerCount = 0;
    bool active = true;

    EntityID id = (EntityID)-1;
};

struct CollectibleComponent
{
    glm::vec3 startPosition;
    float oscillationTimer;

    EntityID id = (EntityID)-1;
};
#endif //PBL_COMPONENTS_H