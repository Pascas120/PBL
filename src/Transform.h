//
// Created by lukas on 31.03.2025.
//

#ifndef TRANSFORM_H
#define TRANSFORM_H
#include <glm/fwd.hpp>
#include <glm/vec3.hpp>
#include "Component.h"
#include <glm/gtc/matrix_transform.hpp>


class Transform : public Component {
private:
    glm::vec3 translate;
    glm::vec3 rotate;
    glm::vec3 scale;
    glm::mat4 modelMatrix;
    glm::mat4 parentMatrix = glm::mat4(1.0f);

public:
    Transform();

    void setTranslation(const glm::vec3& t);
    void setRotation(const glm::vec3& r);
    void setScale(const glm::vec3& s);
    void setParentMatrix(const glm::mat4& m);

    const glm::vec3& getTranslation() const;
    const glm::vec3& getRotation() const;
    const glm::vec3& getScale() const;
    const glm::mat4& getModelMatrix() const;

    void update() override;
};



#endif //TRANSFORM_H
