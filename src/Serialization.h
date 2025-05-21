#pragma once

#include <nlohmann/json.hpp>
#include <vector>
#include "Scene.h"

class Shader;
class Model;

namespace Serialization
{
	struct DeserializationContext {
		std::vector<Shader*>& shaders;
		std::vector<Model*>& models;
		bool deserializeUuid;
	};

	void saveScene(const std::string& filePath, Scene& scene);
	void loadScene(const std::string& filePath, Scene& scene, const DeserializationContext& context);

	nlohmann::json serializeScene(Scene& scene);
	void deserializeScene(nlohmann::json sceneJson, Scene& scene, const DeserializationContext& context);

	nlohmann::json serializeObjects(const std::vector<EntityID>& objects, Scene& scene);
	std::vector<EntityID> deserializeObjects(nlohmann::json objectsJson, Scene& scene, EntityID rootParent, const DeserializationContext& context);

	std::vector<Shader*> loadShaderList(const std::string& filePath, std::vector<Shader*>& shaders);
}