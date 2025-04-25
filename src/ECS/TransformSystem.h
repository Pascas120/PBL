

#ifndef PBL_TRANSFORMSYSTEM_H
#define PBL_TRANSFORMSYSTEM_H

#include "Components.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"

class Scene;

class TransformSystem {
private:
    Scene* scene;

    void updateNode(EntityID id) const;
    void updateNodeRecursive(EntityID id) const;
    void markDirty(EntityID id) const;

public:
    explicit TransformSystem(Scene* scene);

    void update() const;
    void translateEntity(EntityID id, const glm::vec3& translation) const;
    void rotateEntity(EntityID id, const glm::quat& rotation) const;
    void rotateEntity(EntityID id, const glm::vec3& rotation) const;
    void scaleEntity(EntityID id, const glm::vec3& scale) const;
    void setGlobalMatrix(EntityID id, const glm::mat4& mat) const;

    void addChild(EntityID parent, EntityID child) const;
    void removeChild(EntityID parent, EntityID child) const;
};

#endif //PBL_TRANSFORMSYSTEM_H

