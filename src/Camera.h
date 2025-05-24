//
// Created by Łukasz Moskwin on 29/03/2025.
//

#ifndef PBL_CAMERA_H
#define PBL_CAMERA_H


#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <Frustum.h>

struct CamVectorsAngles
{
	glm::vec3 position;
	glm::vec3 forward;
	glm::vec3 up;
	glm::vec3 right;
	float yawDeg;
	float pitchDeg;

	void updateVectors()
	{
		forward = glm::vec3(
            cos(glm::radians(yawDeg)) * cos(glm::radians(pitchDeg)),
			sin(glm::radians(pitchDeg)),
			sin(glm::radians(yawDeg)) * cos(glm::radians(pitchDeg))
        );
		forward = glm::normalize(forward);
		right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
		up = glm::normalize(glm::cross(right, forward));
	}

	glm::mat4 toMatrix() const
	{
		return glm::lookAt(position, position + forward, up);
	}
};

class Camera
{
public:
    Camera(glm::mat4 viewMatrix = glm::mat4(1.0f));

	glm::mat4 getViewMatrix() const { return viewMatrix; }
	glm::mat4 getInvViewMatrix() const { return invViewMatrix; }
    void setViewMatrix(const glm::mat4& view);
    void setInvViewMatrix(const glm::mat4& invView);

	Frustum& getFrustum() { return frustum; }

    CamVectorsAngles getVectorsAndAngles() const;

private:
	glm::mat4 viewMatrix;
	glm::mat4 invViewMatrix;

    Frustum frustum;
};

#endif //PBL_CAMERA_H
