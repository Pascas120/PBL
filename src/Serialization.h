#pragma once

#include <nlohmann/json.hpp>

class Scene;
class Shader;
class Model;

namespace Serialization
{
	struct DeserializationContext {
		std::vector<Shader*>& shaders;
		std::vector<Model*>& models;
	};

	void serializeScene(const std::string& filePath, Scene& scene);
	void deserializeScene(const std::string& filePath, Scene& scene, const DeserializationContext& context);
}