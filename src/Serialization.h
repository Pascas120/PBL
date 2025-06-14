#pragma once

#include <nlohmann/json.hpp>
#include <vector>
#include "Scene.h"

class Shader;
class Model;

namespace Serialization
{
	struct SerializationContext {
		const std::unordered_map<EntityID, std::string>& uuidMap;
	};

	struct GlobalDeserializationContext {
		std::vector<Shader*>& shaders;
		std::vector<Model*>& models;
		std::vector<std::string> sounds;
		bool deserializeUuid;
	};

	struct DeserializationContext {
		std::vector<Shader*>& shaders;
		std::vector<Model*>& models;
		const std::unordered_map<std::string, EntityID>& uuidMap;
		bool deserializeUuid;
	};

	void saveScene(const std::string& filePath, Scene& scene);
	void loadScene(const std::string& filePath, Scene& scene, const GlobalDeserializationContext& context);

	nlohmann::json serializeScene(Scene& scene);
	void deserializeScene(nlohmann::json sceneJson, Scene& scene, const GlobalDeserializationContext& context);

	nlohmann::json serializeObjects(const std::vector<EntityID>& objects, Scene& scene);
	std::vector<EntityID> deserializeObjects(nlohmann::json objectsJson, Scene& scene, EntityID rootParent, const GlobalDeserializationContext& context);

	std::vector<Shader*> loadShaderList(const std::string& filePath, std::vector<Shader*>& shaders);
}