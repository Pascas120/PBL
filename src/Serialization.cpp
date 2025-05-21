#include "Serialization.h"
#include "glm_serialization.h"

#include <spdlog/spdlog.h>
#include <fstream>
#include <algorithm>
#include <stack>
#include <unordered_map>

#include "Scene.h"
#include "ECS/Components.h"
#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"


using json = nlohmann::json;

#define serializeComponent(type) \
	if (scene.hasComponent<type>(entity)) { \
		auto& component = scene.getComponent<type>(entity); \
		to_json(entityJson[std::string(#type)], component); \
	}


#define deserializeComponent(type) \
	if (entityJson.contains(std::string(#type))) { \
		type component; \
		from_json(entityJson[std::string(#type)], component, context); \
		component.id = entity; \
		scene.addComponent<type>(entity, component); \
	}

#define deserializeExistingComponent(type) \
	if (entityJson.contains(std::string(#type))) { \
		type& component = scene.getComponent<type>(entity); \
		from_json(entityJson[std::string(#type)], component, context); \
		component.id = entity; \
	}



namespace Serialization
{
	static void from_json(const nlohmann::json& j, Shader*& sptr, const DeserializationContext& context)
	{
		std::string shaderName = j.get<std::string>();
		auto it = std::find_if(context.shaders.begin(), context.shaders.end(),
			[&](Shader* shader) { return shader->getName() == shaderName; });
		if (it != context.shaders.end())
		{
			sptr = *it;
		}
		else
		{
			sptr = context.shaders[0];
		}
	}

	static void from_json(const nlohmann::json& j, Model*& mptr, const DeserializationContext& context)
	{
		std::string modelPath = j.get<std::string>();
		auto itModel = std::find_if(context.models.begin(), context.models.end(),
			[&](Model* model) { return model->directory == modelPath; });
		if (itModel != context.models.end())
		{
			mptr = *itModel;
		}
		else
		{
			mptr = context.models[0];
		}
	}

	static void to_json(nlohmann::json & j, const ObjectInfoComponent& c)
	{
		j["name"] = c.name;
		j["uuid"] = c.uuid;
	}

	static void from_json(const nlohmann::json& j, ObjectInfoComponent& c, const DeserializationContext& context)
	{
		j.at("name").get_to(c.name);
		if (context.deserializeUuid)
			j.at("uuid").get_to(c.uuid);
	}

	static void to_json(nlohmann::json& j, const Transform& c)
	{
		j["translation"] = c.translation;
		j["rotation"] = c.rotation;
		j["eulerRotation"] = c.eulerRotation;
		j["scale"] = c.scale;
	}

	static void from_json(const nlohmann::json& j, Transform& c, const DeserializationContext& context)
	{
		j.at("translation").get_to(c.translation);
		j.at("rotation").get_to(c.rotation);
		j.at("eulerRotation").get_to(c.eulerRotation);
		j.at("scale").get_to(c.scale);
	}

	static void to_json(nlohmann::json& j, const ModelComponent& c)
	{
		j["shader"] = c.shader->getName();
		j["model"] = c.model->directory;
	}

	static void from_json(const nlohmann::json& j, ModelComponent& c, const DeserializationContext& context)
	{
		from_json(j.at("shader"), c.shader, context);
		from_json(j.at("model"), c.model, context);
	}

	static void to_json(nlohmann::json& j, const ImageComponent& c)
	{
		j["shader"] = c.shader->getName();
		j["texturePath"] = c.texturePath;
		j["width"] = c.width;
		j["height"] = c.height;
		j["color"] = c.color;
	}

	static void from_json(const nlohmann::json& j, ImageComponent& c, const DeserializationContext& context)
	{
		from_json(j.at("shader"), c.shader, context);
		j.at("texturePath").get_to(c.texturePath);
		j.at("width").get_to(c.width);
		j.at("height").get_to(c.height);
		j.at("color").get_to(c.color);
	}

	static void to_json(nlohmann::json& j, const TextComponent& c)
	{
		j["shader"] = c.shader->getName();
		j["font"] = c.font;
		j["color"] = c.color;
		j["text"] = c.text;
	}

	static void from_json(const nlohmann::json& j, TextComponent& c, const DeserializationContext& context)
	{
		from_json(j.at("shader"), c.shader, context);
		j.at("font").get_to(c.font);
		j.at("color").get_to(c.color);
		j.at("text").get_to(c.text);
	}

	static void to_json(nlohmann::json& j, const ColliderComponent& c)
	{
		ColliderShape* shape = c.GetColliderShape();
		if (shape)
		{
			json shapeJson;
			if (shape->getType() == ColliderType::BOX)
			{
				auto box = static_cast<BoxCollider*>(shape);
				shapeJson["type"] = "BOX";
				shapeJson["halfSize"] = box->halfSize;
			}
			else if (shape->getType() == ColliderType::SPHERE)
			{
				auto sphere = static_cast<SphereCollider*>(shape);
				shapeJson["type"] = "SPHERE";
				shapeJson["radius"] = sphere->radius;
			}
			shapeJson["center"] = shape->center;
			j["colliderShape"] = shapeJson;
		}
		j["isStatic"] = c.isStatic;
	}

	static void from_json(const nlohmann::json& j, ColliderComponent& c, const DeserializationContext& context)
	{
		static const std::unordered_map<std::string, ColliderType> colliderTypeMap = {
			{"BOX", ColliderType::BOX},
			{"SPHERE", ColliderType::SPHERE}
		};

		if (j.contains("colliderShape"))
		{
			auto shapeJson = j.at("colliderShape");
			std::string type = shapeJson.at("type").get<std::string>();
			auto it = colliderTypeMap.find(type);
			if (it != colliderTypeMap.end())
			{
				c = ColliderComponent(it->second);
				if (it->second == ColliderType::BOX)
				{
					auto box = static_cast<BoxCollider*>(c.GetColliderShape());
					shapeJson.at("halfSize").get_to(box->halfSize);
					shapeJson.at("center").get_to(box->center);
				}
				else if (it->second == ColliderType::SPHERE)
				{
					auto sphere = static_cast<SphereCollider*>(c.GetColliderShape());
					shapeJson.at("radius").get_to(sphere->radius);
					shapeJson.at("center").get_to(sphere->center);
				}

				c.isStatic = j.at("isStatic").get<bool>();
			}
		}
	}

	void saveScene(const std::string& filePath, Scene& scene)
	{
		std::ofstream file(filePath);
		if (file.is_open())
		{
			file << serializeScene(scene).dump(4);
			file.close();
		}
		else
		{
			spdlog::error("Failed to open file for writing: {}", filePath);
		}
	}

	void loadScene(const std::string& filePath, Scene& scene, const DeserializationContext& context)
	{
		std::ifstream file(filePath);
		if (file.is_open())
		{
			json sceneJson;
			file >> sceneJson;
			file.close();
			deserializeScene(sceneJson, scene, context);
		}
		else
		{
			spdlog::error("Failed to open file for reading: {}", filePath);
		}
	}




	json serializeScene(Scene& scene)
	{
		json sceneJson;

		//auto& entities = scene.getEntities();
		EntityID sceneRoot = scene.getSceneRootEntity();

		sceneJson["sceneRoot"] = scene.getComponent<ObjectInfoComponent>(sceneRoot).name;

		sceneJson["entities"] = serializeObjects({ sceneRoot }, scene)["entities"];

		return sceneJson;
	}

	void deserializeScene(json sceneJson, Scene& scene, const DeserializationContext& context)
	{
		EntityID sceneRoot = scene.getSceneRootEntity();

		scene.getComponent<ObjectInfoComponent>(sceneRoot).name = sceneJson["sceneRoot"].get<std::string>();

		deserializeObjects(sceneJson, scene, sceneRoot, context);
	}

	struct FullEntitySelection
	{
		std::vector<EntityID> selectedEntities;
		std::unordered_set<EntityID> roots;
	};

	static FullEntitySelection getSelectedTree(const std::vector<EntityID>& selection, Scene& scene)
	{
		FullEntitySelection result;
		EntityID sceneRoot = scene.getSceneRootEntity();
		result.roots.insert(sceneRoot);

		std::stack<EntityID> stack;
		for (auto it = selection.rbegin(); it != selection.rend(); ++it)
		{
			stack.push(*it);
		}
		std::unordered_set<EntityID> visited;

		while (!stack.empty())
		{
			EntityID id = stack.top();
			stack.pop();
			if (visited.find(id) != visited.end())
				continue;

			visited.insert(id);

			if (id != sceneRoot)
				result.selectedEntities.push_back(id);

			auto& transform = scene.getComponent<Transform>(id);
			for (auto it = transform.children.rbegin(); it != transform.children.rend(); ++it)
			{
				if (visited.find(*it) == visited.end())
					stack.push(*it);
			}
		}

		for (auto& id : selection)
		{
			if (id == sceneRoot)
				continue;

			auto& transform = scene.getComponent<Transform>(id);
			if (visited.find(transform.parent) == visited.end())
			{
				result.roots.insert(transform.parent);
			}
		}


		return result;
	}

	json serializeObjects(const std::vector<EntityID>& objects, Scene& scene)
	{
		auto [entities, roots] = getSelectedTree(objects, scene);
		json selectionJson;

		for (auto& entity : entities)
		{
			json entityJson;
			serializeComponent(ObjectInfoComponent);
			serializeComponent(Transform);
			auto& t = scene.getComponent<Transform>(entity);
			if (roots.find(t.parent) != roots.end())
				entityJson["Transform"]["parent"] = "";
			else
				entityJson["Transform"]["parent"] = scene.getComponent<ObjectInfoComponent>(t.parent).uuid;

			entityJson["Transform"]["children"] = json::array();
			for (auto& child : t.children)
			{
				entityJson["Transform"]["children"].push_back(scene.getComponent<ObjectInfoComponent>(child).uuid);
			}


			serializeComponent(ModelComponent);
			serializeComponent(ImageComponent);
			serializeComponent(TextComponent);
			serializeComponent(ColliderComponent);



			selectionJson["entities"].push_back(entityJson);
		}

		return selectionJson;
	}

	std::vector<EntityID> deserializeObjects(nlohmann::json objectsJson, Scene& scene, EntityID rootParent, const DeserializationContext& context)
	{
		std::unordered_map<std::string, EntityID> uuidToEntityMap = { {"", rootParent} };
		std::vector<EntityID> deserializedEntities;
		for (const auto& entityJson : objectsJson["entities"])
		{
			EntityID entity = scene.createEntity((EntityID)-1);
			uuidToEntityMap[entityJson["ObjectInfoComponent"]["uuid"].get<std::string>()] = entity;
			deserializedEntities.push_back(entity);


			deserializeExistingComponent(ObjectInfoComponent);
			deserializeExistingComponent(Transform);

			deserializeComponent(ModelComponent);
			deserializeComponent(ImageComponent);
			deserializeComponent(TextComponent);
			deserializeComponent(ColliderComponent);
		}

		auto& ts = scene.getTransformSystem();

		for (auto& entityJson : objectsJson["entities"])
		{
			EntityID entity = uuidToEntityMap[entityJson["ObjectInfoComponent"]["uuid"].get<std::string>()];
			auto& t = scene.getComponent<Transform>(entity);

			ts.addChild(uuidToEntityMap[entityJson["Transform"]["parent"].get<std::string>()],
				entity);

			for (auto& childUuid : entityJson["Transform"]["children"])
			{
				ts.addChild(entity, uuidToEntityMap[childUuid.get<std::string>()]);
			}
		}

		return deserializedEntities;
	}

	static const std::unordered_map<std::string, GLenum> shaderTypeMap = {
		{"VERTEX", GL_VERTEX_SHADER},
		{"FRAGMENT", GL_FRAGMENT_SHADER},
		{"GEOMETRY", GL_GEOMETRY_SHADER},
		{"TESS_CONTROL", GL_TESS_CONTROL_SHADER},
		{"TESS_EVALUATION", GL_TESS_EVALUATION_SHADER},
		{"COMPUTE", GL_COMPUTE_SHADER}
	};

	std::vector<Shader*> Serialization::loadShaderList(const std::string& filePath, std::vector<Shader*>& shaders)
	{
		std::vector<Shader*> newShaders;

		std::ifstream file(filePath);
		if (file.is_open())
		{
			json shaderListJson;
			file >> shaderListJson;
			file.close();
			for (const auto& shaderJson : shaderListJson["shaders"])
			{
				std::unordered_map<GLenum, std::string> shaderPaths;
				for (const auto& source : shaderJson["sources"].items())
				{
					std::string type = source.key();
					std::string path = source.value();
					auto it = shaderTypeMap.find(type);
					if (it != shaderTypeMap.end())
					{
						shaderPaths[it->second] = path;
					}
				}

				Shader* shader = new Shader(shaderJson["name"].get<std::string>(), shaderPaths);
				shaders.push_back(shader);
				newShaders.push_back(shader);
			}
		}
		else
		{
			spdlog::error("Failed to open file for reading: {}", filePath);
		}
		return newShaders;
	}
}