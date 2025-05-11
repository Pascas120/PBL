#include "Serialization.h"
#include "glm_serialization.h"

#include <spdlog/spdlog.h>
#include <fstream>
#include <algorithm>

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
	}

	static void from_json(const nlohmann::json& j, ObjectInfoComponent& c, const DeserializationContext& context)
	{
		j.at("name").get_to(c.name);
	}

	static void to_json(nlohmann::json& j, const Transform& c)
	{
		j["translation"] = c.translation;
		j["rotation"] = c.rotation;
		j["eulerRotation"] = c.eulerRotation;
		j["scale"] = c.scale;
		j["uuid"] = c.uuid;
	}

	static void from_json(const nlohmann::json& j, Transform& c, const DeserializationContext& context)
	{
		j.at("translation").get_to(c.translation);
		j.at("rotation").get_to(c.rotation);
		j.at("eulerRotation").get_to(c.eulerRotation);
		j.at("scale").get_to(c.scale);
		j.at("uuid").get_to(c.uuid);
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
				}
				else if (it->second == ColliderType::SPHERE)
				{
					auto sphere = static_cast<SphereCollider*>(c.GetColliderShape());
					shapeJson.at("radius").get_to(sphere->radius);
				}

				c.isStatic = j.at("isStatic").get<bool>();
			}
		}
	}

	static void to_json(nlohmann::json& j, const BoundingVolumeComponent& c)
	{
		// temporary
		j = c.GetBoundingVolume()->serialize();
	}

	static void from_json(const nlohmann::json& j, BoundingVolumeComponent& c, const DeserializationContext& context)
	{
		std::string type = j.at("type").get<std::string>();
		if (type == "SPHERE")
		{
			glm::vec3 center;
			float radius;
			j.at("center").get_to(center);
			j.at("radius").get_to(radius);
			c = BoundingVolumeComponent(std::make_unique<SphereBV>(center, radius));
		}
		else if (type == "AABB")
		{
			glm::vec3 center, extents;
			j.at("center").get_to(center);
			j.at("extents").get_to(extents);
			c = BoundingVolumeComponent(std::make_unique<AABBBV>(center, extents.x, extents.y, extents.z));
		}
	}


	void serializeScene(const std::string& filePath, Scene& scene)
	{
		json sceneJson;

		auto& entities = scene.getEntities();
		EntityID sceneRoot = scene.getSceneRootEntity();

		sceneJson["sceneRoot"] = scene.getComponent<ObjectInfoComponent>(sceneRoot).name;

		for (auto& entity : entities)
		{
			if (entity == sceneRoot)
				continue;

			json entityJson;
			serializeComponent(ObjectInfoComponent);
			serializeComponent(Transform);
			auto& t = scene.getComponent<Transform>(entity);
			if (t.parent == sceneRoot)
				entityJson["Transform"]["parent"] = "";
			else
				entityJson["Transform"]["parent"] = scene.getComponent<Transform>(t.parent).uuid;

			entityJson["Transform"]["children"] = json::array();
			for (auto& child : t.children)
			{
				entityJson["Transform"]["children"].push_back(scene.getComponent<Transform>(child).uuid);
			}


			serializeComponent(ModelComponent);
			serializeComponent(ImageComponent);
			serializeComponent(TextComponent);
			serializeComponent(ColliderComponent);
			serializeComponent(BoundingVolumeComponent);



			sceneJson["entities"].push_back(entityJson);
		}

		std::ofstream file(filePath);
		if (file.is_open())
		{
			file << sceneJson.dump(4);
			file.close();
		}
		else
		{
			spdlog::error("Failed to open file for writing: {}", filePath);
		}
	}

	void deserializeScene(const std::string& filePath, Scene& scene, const DeserializationContext& context)
	{
		std::ifstream file(filePath);
		if (!file.is_open())
		{
			spdlog::error("Failed to open file for reading: {}", filePath);
			return;
		}

		json sceneJson;
		file >> sceneJson;
		file.close();

		EntityID sceneRoot = scene.getSceneRootEntity();

		scene.getComponent<ObjectInfoComponent>(sceneRoot).name = sceneJson["sceneRoot"].get<std::string>();

		std::unordered_map<std::string, EntityID> uuidToEntityMap = { {"", sceneRoot} };
		for (const auto& entityJson : sceneJson["entities"])
		{
			EntityID entity = scene.createEntity((EntityID)-1);
			
			deserializeExistingComponent(ObjectInfoComponent);
			spdlog::info("Deserializing entity: {}", scene.getComponent<ObjectInfoComponent>(entity).name);
			deserializeExistingComponent(Transform);
			auto& t = scene.getComponent<Transform>(entity);
			uuidToEntityMap[t.uuid] = entity;

			deserializeComponent(ModelComponent);
			deserializeComponent(ImageComponent);
			deserializeComponent(TextComponent);
			deserializeComponent(ColliderComponent);
			deserializeComponent(BoundingVolumeComponent);
		}

		for (auto& entityJson : sceneJson["entities"])
		{
			EntityID entity = uuidToEntityMap[entityJson["Transform"]["uuid"].get<std::string>()];
			auto& t = scene.getComponent<Transform>(entity);
			t.parent = uuidToEntityMap[entityJson["Transform"]["parent"].get<std::string>()];

			for (auto& childUuid : entityJson["Transform"]["children"])
			{
				t.children.push_back(uuidToEntityMap[childUuid.get<std::string>()]);
			}
		}
	}
}