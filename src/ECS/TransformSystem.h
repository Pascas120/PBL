

#ifndef PBL_TRANSFORMSYSTEM_H
#define PBL_TRANSFORMSYSTEM_H

#include "Components.h"
#include "Scene.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"

class TransformSystem {
private:
    Scene* scene;

    void updateNode(EntityID id);
    void updateNodeRecursive(EntityID id);
    void markDirty(EntityID id);

public:
    explicit TransformSystem(Scene* scene);

    void update();
    void translateEntity(EntityID id, const glm::vec3& translation);
    void rotateEntity(EntityID id, const glm::quat& rotation);
    void scaleEntity(EntityID id, const glm::vec3& scale);
};

#endif //PBL_TRANSFORMSYSTEM_H

