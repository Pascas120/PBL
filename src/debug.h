#pragma once

#include <glm/glm.hpp>
class ColliderShape;
class Transform;
#include "Shader.h"


namespace Debug
{
	void Init(Shader* shader);
	void DrawCollider(ColliderShape* collider, Transform* transform, const glm::vec3& color = glm::vec3(1.0f, 0.0f, 0.0f));
	void SetProjectionView(const glm::mat4& projection, const glm::mat4& view);
}