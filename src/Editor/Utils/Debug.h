#pragma once

#include <glm/glm.hpp>
#include "Shader.h"

#include <memory>

#include "ECS/Components.h"
class Shader;
class Mesh;

namespace Editor::Utils
{
	class Debug
	{
	public:
		Debug();
		//void setProjectionView(glm::mat4 projection, glm::mat4 view);
		void drawCollider(ColliderShape* collider, const Transform& transform, Shader* shader,
			const glm::vec3& color = glm::vec3(1.0f, 0.0f, 0.0f)) const;

		//void setRect(float x, float y, float width, float height);


	private:
		std::shared_ptr<Mesh> boxMesh;
		std::shared_ptr<Mesh> sphereMesh;

		//float x, y, width, height;

	};
}