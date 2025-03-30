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

public:
    Transform();

    void SetTranslation(const glm::vec3& t);
    void SetRotation(const glm::vec3& r);
    void SetScale(const glm::vec3& s);

    const glm::vec3& GetTranslation() const;
    const glm::vec3& GetRotation() const;
    const glm::vec3& GetScale() const;
    const glm::mat4& GetModelMatrix() const;

    void update() override;
};



#endif //TRANSFORM_H
