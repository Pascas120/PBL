#include "SplitScreenController.h"
#include "Scene.h"
#include "spdlog/spdlog.h"

void SplitScreenController::update(GLFWwindow* window, Scene* scene, float deltaTime) {
	if (target1 == (EntityID)-1 || target2 == (EntityID)-1 ||
		camera1 == (EntityID)-1 || camera2 == (EntityID)-1) {
		return;
	}
	if (!scene->hasComponent<CameraComponent>(camera1) ||
		!scene->hasComponent<CameraComponent>(camera2)) {
		return;
	}

	auto& ts = scene->getTransformSystem();
	Transform& targetTransform1 = scene->getComponent<Transform>(target1);
	Transform& targetTransform2 = scene->getComponent<Transform>(target2);
	Transform& sscTransform = scene->getComponent<Transform>(id);
	ts.translateEntity(id, (targetTransform1.translation + targetTransform2.translation) / 2.0f);


	glm::vec3 direction = -glm::normalize(offset);
	glm::vec3 up = { 0.0f, 1.0f, 0.0f }; // Assuming Y is up
	glm::vec3 right = glm::normalize(glm::cross(direction, up));
	up = glm::normalize(glm::cross(right, direction));
	glm::quat rotation = glm::quatLookAt(direction, up);

	glm::vec3 cameraPos = sscTransform.translation + offset;
	
	float dist = glm::distance(targetTransform1.translation, targetTransform2.translation);


	if (dist < 8.0f)
	{
		ts.translateEntity(camera1, cameraPos);
		ts.translateEntity(camera2, cameraPos);

		splitActive = false;
	}
	else
	{
		glm::vec3 diff3 = targetTransform2.translation - targetTransform1.translation;
		glm::vec2 diff = glm::vec2(diff3.x, diff3.z);

		if (diff.y == 0.0f)
		{
			splitSlope = 100000.0f;
		}
		else
		{
			splitSlope = -diff.x / diff.y;
		}


		float mixFactor = glm::smoothstep(8.0f, 12.0f, dist);
		glm::vec3 camera1TagetPos = targetTransform1.translation + offset;
		glm::vec3 camera2TargetPos = targetTransform2.translation + offset;

		glm::vec3 newCamera1Pos = glm::mix(cameraPos, camera1TagetPos, mixFactor);
		glm::vec3 newCamera2Pos = glm::mix(cameraPos, camera2TargetPos, mixFactor);

		ts.translateEntity(camera1, newCamera1Pos);
		ts.translateEntity(camera2, newCamera2Pos);

		auto& cam1Component = scene->getComponent<CameraComponent>(camera1);
		auto& cam2Component = scene->getComponent<CameraComponent>(camera2);

		glm::vec2 splitNdc1 = glm::normalize(diff) * 0.5f;
		glm::vec2 splitNdc2 = -splitNdc1;

		cam1Component.screenOffset = splitNdc1 * mixFactor;
		cam2Component.screenOffset = splitNdc2 * mixFactor;

		cam1Component.updateProjectionMatrix();
		cam2Component.updateProjectionMatrix();

		target1AboveSlope = (targetTransform1.translation.z - 
			(splitSlope * (targetTransform1.translation.x - sscTransform.translation.x) + sscTransform.translation.z)) < 0.0f;
		splitActive = true;
		splitLineThickness = 5.0f * mixFactor;
	}
	ts.rotateEntity(camera1, rotation);
	ts.rotateEntity(camera2, rotation);


	/*clip = projection * centerView * glm::vec4(targetTransform2.translation, 1.0f);
	glm::vec3 ndc2 = glm::vec3(clip) / clip.w;*/

	

}

