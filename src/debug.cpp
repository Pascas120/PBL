#include "debug.h"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <Mesh.h>
#include <vector>

#include "Shader.h"
#include "ColliderComponent.h"
#include "Transform.h"

// :)
#include <memory>


static std::vector<Vertex> pointsToVertices(const std::vector<glm::vec3>& points);
static std::shared_ptr<Mesh> createSphereMesh();

static bool initialized = false;
static Shader debugShader;
static std::shared_ptr<Mesh> boxMesh;

static std::shared_ptr<Mesh> sphereMesh;

void Debug::Init()
{
	debugShader = Shader("res/shaders/flat.vert", "res/shaders/flat.frag");
	boxMesh = std::shared_ptr<Mesh>(new Mesh(
		// vertices
		pointsToVertices({
			{-1, -1, -1}, {1, -1, -1}, {1, 1, -1}, {-1, 1, -1},
			{-1, -1,  1}, {1, -1,  1}, {1, 1,  1}, {-1, 1,  1},
			}),
		// indices
		{
			0,1, 1,2, 2,3, 3,0,
			4,5, 5,6, 6,7, 7,4,
			0,4, 1,5, 2,6, 3,7,
		},
		// textures
		{}
		));
	boxMesh->SetDrawMode(GL_LINES);
		
	sphereMesh = createSphereMesh();
	sphereMesh->SetDrawMode(GL_LINES);

	initialized = true;

}

void Debug::SetProjectionView(const glm::mat4& projection, const glm::mat4& view)
{
	if (!initialized) return;
	debugShader.use();
	debugShader.setMat4("projection", projection);
	debugShader.setMat4("view", view);
}

void Debug::DrawCollider(ColliderShape* collider, Transform* transform, const glm::vec3& color)
{
	if (!initialized) return;
	ColliderType type = collider->getType();
	
	glm::mat4 model = transform->getModelMatrix();
	glm::mat4 colliderMatrix = glm::mat4(1.0f);
	std::shared_ptr<Mesh> mesh = nullptr;
	switch (type)
	{
	case BOX:
	{
		BoxCollider* boxCollider = static_cast<BoxCollider*>(collider);
		colliderMatrix = glm::translate(model, boxCollider->center);
		colliderMatrix = glm::scale(colliderMatrix, boxCollider->halfSize);
		mesh = boxMesh;
	}
		break;
	case SPHERE:
	{
		glm::vec3 scale;
		scale.x = glm::length(glm::vec3(model[0]));
		scale.y = glm::length(glm::vec3(model[1]));
		scale.z = glm::length(glm::vec3(model[2]));

		float maxScale = std::max(scale.x, std::max(scale.y, scale.z));

		model[0] = glm::vec4(glm::normalize(glm::vec3(model[0])) * maxScale, 0.0f);
		model[1] = glm::vec4(glm::normalize(glm::vec3(model[1])) * maxScale, 0.0f);
		model[2] = glm::vec4(glm::normalize(glm::vec3(model[2])) * maxScale, 0.0f);

		SphereCollider* sphereCollider = static_cast<SphereCollider*>(collider);
		colliderMatrix = glm::translate(model, sphereCollider->center);
		colliderMatrix = glm::scale(colliderMatrix, glm::vec3(sphereCollider->radius));

		mesh = sphereMesh;
	}
		break;
	case CAPSULE:
	default:
		return;
	}

	debugShader.use();
	debugShader.setVec3("color", color);
	debugShader.setMat4("model", colliderMatrix);
	mesh->Draw(debugShader);
}



static std::vector<Vertex> pointsToVertices(const std::vector<glm::vec3>& points)
{
	std::vector<Vertex> vertices;
	vertices.reserve(points.size());
	for (const auto& point : points)
	{
		Vertex vertex;
		vertex.Position = point;
		vertices.push_back(vertex);
	}
	return vertices;
}

static std::shared_ptr<Mesh> createSphereMesh()
{
	constexpr int numSegments = 36;

	std::vector<glm::vec3> points(numSegments * 3);
	std::vector<unsigned int> indices(numSegments * 6);

	// 3 axis aligned circles
	for (int i = 0; i < numSegments; i++)
	{
		float theta = (float)i / (float)numSegments * 2.0f * glm::pi<float>();
		float x = glm::cos(theta);
		float y = glm::sin(theta);

		points[i] = { x, y, 0 };
		points[i + numSegments] = { x, 0, y };
		points[i + numSegments * 2] = { 0, x, y };

		unsigned int first = i, second = (i + 1) % numSegments;

		indices[i * 6 + 0] = first;
		indices[i * 6 + 1] = second;

		indices[i * 6 + 2] = first + numSegments;
		indices[i * 6 + 3] = second + numSegments;

		indices[i * 6 + 4] = first + (2 * numSegments);
		indices[i * 6 + 5] = second + (2 * numSegments);
	}

	return std::shared_ptr<Mesh>(new Mesh(
		pointsToVertices(points),
		indices,
		{}
	)
	);
}

