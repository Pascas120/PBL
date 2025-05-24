//
// Created by Łukasz Moskwin on 29/03/2025.
//

#include "Camera.h"




Camera::Camera(glm::mat4 viewMatrix)
{
	setViewMatrix(viewMatrix);
}

void Camera::setViewMatrix(const glm::mat4& view)
{
	viewMatrix = view;
	invViewMatrix = glm::inverse(view);
}

void Camera::setInvViewMatrix(const glm::mat4& invView)
{
    invViewMatrix = invView;
    viewMatrix = glm::inverse(invView);
}

CamVectorsAngles Camera::getVectorsAndAngles() const
{
    CamVectorsAngles vectorsAngles;
    vectorsAngles.position = glm::vec3(invViewMatrix[3]);
    vectorsAngles.forward = -glm::vec3(invViewMatrix[2]);
    vectorsAngles.up = glm::vec3(invViewMatrix[1]);
    vectorsAngles.right = glm::vec3(invViewMatrix[0]);

    vectorsAngles.yawDeg = atan2(vectorsAngles.forward.z, vectorsAngles.forward.x);
	vectorsAngles.yawDeg = glm::degrees(vectorsAngles.yawDeg);
    vectorsAngles.pitchDeg = asin(vectorsAngles.forward.y);
	vectorsAngles.pitchDeg = glm::degrees(vectorsAngles.pitchDeg);

	return vectorsAngles;
}
