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
		to_json(entityJson[std::string(#type)], component, context); \
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
			[&](Model* model) { return model->path == modelPath; });
		if (itModel != context.models.end())
		{
			mptr = *itModel;
		}
		else
		{
			mptr = context.models[0];
		}
	}


	static json entity_to_json(EntityID entity, const SerializationContext& context)
	{
		json j;
		auto it = context.uuidMap.find(entity);
		if (it != context.uuidMap.end())
		{
			j = it->second;
		}
		else
		{
			j = "";
		}
		return j;
	}

	static EntityID entity_from_json(const json& j, const DeserializationContext& context)
	{
		if (j.is_string() && !j.get<std::string>().empty())
		{
			auto it = context.uuidMap.find(j.get<std::string>());
			if (it != context.uuidMap.end())
			{
				return it->second;
			}
		}
		return (EntityID)-1;
	}


	static void to_json(nlohmann::json & j, const ObjectInfoComponent& c, const SerializationContext& context)
	{
		j["name"] = c.name;
		j["uuid"] = c.uuid;
		if (!c.tag.empty())
		{
			j["tag"] = c.tag;
		}
	}

	static void from_json(const nlohmann::json& j, ObjectInfoComponent& c, const DeserializationContext& context)
	{
		j.at("name").get_to(c.name);
		if (context.deserializeUuid)
			j.at("uuid").get_to(c.uuid);

		if (j.contains("tag"))
		{
			j.at("tag").get_to(c.tag);
		}
		else
		{
			c.tag.clear();
		}
	}

	static void to_json(nlohmann::json& j, const Transform& c, const SerializationContext& context)
	{
		j["isStatic"] = c.isStatic;
		j["translation"] = c.translation;
		j["rotation"] = c.rotation;
		j["eulerRotation"] = c.eulerRotation;
		j["scale"] = c.scale;
		j["parent"] = entity_to_json(c.parent, context);
		j["children"] = nlohmann::json::array();
		for (const auto& child : c.children)
		{
			j["children"].push_back(entity_to_json(child, context));
		}

	}

	static void from_json(const nlohmann::json& j, Transform& c, const DeserializationContext& context)
	{
		j.at("isStatic").get_to(c.isStatic);
		j.at("translation").get_to(c.translation);
		j.at("rotation").get_to(c.rotation);
		j.at("eulerRotation").get_to(c.eulerRotation);
		j.at("scale").get_to(c.scale);
		c.parent = entity_from_json(j.at("parent"), context);
		c.children.clear();
		for (const auto& child : j.at("children"))
		{
			EntityID childId = entity_from_json(child, context);
			if (childId != (EntityID)-1)
			{
				c.children.push_back(childId);
			}
		}
	}

	static void to_json(nlohmann::json& j, const ModelComponent& c, const SerializationContext& context)
	{
		j["shader"] = c.shader->getName();
		j["model"] = c.model->path;
	}

	static void from_json(const nlohmann::json& j, ModelComponent& c, const DeserializationContext& context)
	{
		from_json(j.at("shader"), c.shader, context);
		from_json(j.at("model"), c.model, context);
	}

	static void to_json(nlohmann::json& j, const ImageComponent& c, const SerializationContext& context)
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

	static void to_json(nlohmann::json& j, const TextComponent& c, const SerializationContext& context)
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

	static void to_json(nlohmann::json& j, const ColliderComponent& c, const SerializationContext& context)
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


	static void to_json(nlohmann::json& j, const CameraComponent& c, const SerializationContext& context)
	{
		j["aspectRatio"] = c.aspectRatio;
		j["nearPlane"] = c.nearPlane;
		j["farPlane"] = c.farPlane;
		j["screenOffset"] = c.screenOffset;

		j["projectionType"] = c.projectionType == CameraComponent::ProjectionType::PERSPECTIVE ? "PERSPECTIVE" : "ORTHOGRAPHIC";
		json jPerspective, jOrthographic;

		jPerspective["fov"] = c.perspective.fov;
		j["perspective"] = jPerspective;

		jOrthographic["size"] = c.orthographic.size;
		j["orthographic"] = jOrthographic;

		j["fovSizeAxis"] = c.fovSizeAxis == CameraComponent::FovSizeAxis::VERTICAL ? "VERTICAL" : "HORIZONTAL";
	}

	static void from_json(const nlohmann::json& j, CameraComponent& c, const DeserializationContext& context)
	{
		j.at("aspectRatio").get_to(c.aspectRatio);
		j.at("nearPlane").get_to(c.nearPlane);
		j.at("farPlane").get_to(c.farPlane);
		j.at("screenOffset").get_to(c.screenOffset);

		std::string projectionType = j.at("projectionType").get<std::string>();
		if (projectionType == "PERSPECTIVE")
			c.projectionType = CameraComponent::ProjectionType::PERSPECTIVE;
		else if (projectionType == "ORTHOGRAPHIC")
			c.projectionType = CameraComponent::ProjectionType::ORTHOGRAPHIC;

		c.perspective.fov = j.at("perspective").at("fov").get<float>();

		c.orthographic.size = j.at("orthographic").at("size").get<float>();

		std::string fovSizeAxis = j.at("fovSizeAxis").get<std::string>();
		if (fovSizeAxis == "VERTICAL")
			c.fovSizeAxis = CameraComponent::FovSizeAxis::VERTICAL;
		else if (fovSizeAxis == "HORIZONTAL")
			c.fovSizeAxis = CameraComponent::FovSizeAxis::HORIZONTAL;
	}

	static void to_json(nlohmann::json& j, PointLightComponent& c, const SerializationContext& context)
	{
		j["color"] = c.color;
		j["intensity"] = c.intensity;
		j["constant"] = c.constant;
		j["linear"] = c.linear;
		j["quadratic"] = c.quadratic;
	}

	static void from_json(const nlohmann::json& j, PointLightComponent& c, const DeserializationContext& context)
	{
		j.at("color").get_to(c.color);
		j.at("intensity").get_to(c.intensity);
		j.at("constant").get_to(c.constant);
		j.at("linear").get_to(c.linear);
		j.at("quadratic").get_to(c.quadratic);
	}

	static void to_json(nlohmann::json& j, const DirectionalLightComponent& c, const SerializationContext& context)
	{
		j["color"] = c.color;
		j["intensity"] = c.intensity;
	}

	static void from_json(const nlohmann::json& j, DirectionalLightComponent& c, const DeserializationContext& context)
	{
		j.at("color").get_to(c.color);
		j.at("intensity").get_to(c.intensity);
	}

	/*
	struct FlyAIComponent {
	EntityID idButter = (EntityID)-1;
    EntityID idBread = (EntityID)-1;
    float patrolHeightOffset = 3.f;
    float patrolSpeed = 3.f;
    float diveSpeed = 6.f;
    float detectionRadius = 8.f;
    float diveEndHeight = 1.f;
    float returnSpeed = 3.5f;
    float patrolRange = 10.f;
    float patrolPointReachedThreshold = 0.5f;
    float diveCooldownTime = 2.f;
    float diveCooldownTimer = 0.f;
	float groundY = 0.f;
    enum FlyState { Patrolling, Diving, Returning }
    state = FlyState::Patrolling;
    glm::vec3 patrolTarget;

    EntityID id = (EntityID)-1;


};
	*/

	static void to_json(nlohmann::json& j, const FlyAIComponent& c, const SerializationContext& context)
	{
		j["idButter"] = entity_to_json(c.idButter, context);
		j["idBread"] = entity_to_json(c.idBread, context);
		j["patrolHeightOffset"] = c.patrolHeightOffset;
		j["patrolSpeed"] = c.patrolSpeed;
		j["diveSpeed"] = c.diveSpeed;
		j["detectionRadius"] = c.detectionRadius;
		j["diveEndHeight"] = c.diveEndHeight;
		j["returnSpeed"] = c.returnSpeed;
		j["patrolRange"] = c.patrolRange;
		j["patrolPointReachedThreshold"] = c.patrolPointReachedThreshold;
		j["diveCooldownTime"] = c.diveCooldownTime;
		j["diveCooldownTimer"] = c.diveCooldownTimer;
		j["groundY"] = c.groundY;
		std::string stateStr;
		switch (c.state)
		{
		case FlyAIComponent::Patrolling: stateStr = "Patrolling"; break;
		case FlyAIComponent::Diving: stateStr = "Diving"; break;
		case FlyAIComponent::Returning: stateStr = "Returning"; break;
		}
		j["state"] = stateStr;
		j["patrolTarget"] = c.patrolTarget;
	}

	static void from_json(const nlohmann::json& j, FlyAIComponent& c, const DeserializationContext& context)
	{
		c.idButter = entity_from_json(j.at("idButter"), context);
		c.idBread = entity_from_json(j.at("idBread"), context);
		j.at("patrolHeightOffset").get_to(c.patrolHeightOffset);
		j.at("patrolSpeed").get_to(c.patrolSpeed);
		j.at("diveSpeed").get_to(c.diveSpeed);
		j.at("detectionRadius").get_to(c.detectionRadius);
		j.at("diveEndHeight").get_to(c.diveEndHeight);
		j.at("returnSpeed").get_to(c.returnSpeed);
		j.at("patrolRange").get_to(c.patrolRange);
		j.at("patrolPointReachedThreshold").get_to(c.patrolPointReachedThreshold);
		j.at("diveCooldownTime").get_to(c.diveCooldownTime);
		j.at("diveCooldownTimer").get_to(c.diveCooldownTimer);
		j.at("groundY").get_to(c.groundY);
		std::string stateStr = j.at("state").get<std::string>();
		if (stateStr == "Patrolling") c.state = FlyAIComponent::Patrolling;
		else if (stateStr == "Diving") c.state = FlyAIComponent::Diving;
		else if (stateStr == "Returning") c.state = FlyAIComponent::Returning;
		j.at("patrolTarget").get_to(c.patrolTarget);
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

	void loadScene(const std::string& filePath, Scene& scene, const GlobalDeserializationContext& context)
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

		EntityID sceneRoot = scene.getSceneRootEntity();

		sceneJson["sceneRoot"] = scene.getComponent<ObjectInfoComponent>(sceneRoot).name;

		sceneJson["entities"] = serializeObjects({ sceneRoot }, scene)["entities"];

		return sceneJson;
	}

	void deserializeScene(json sceneJson, Scene& scene, const GlobalDeserializationContext& context)
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
		// rooty chyba ju� nie s� potrzebne
		auto [entities, roots] = getSelectedTree(objects, scene);
		json selectionJson;

		std::unordered_map<EntityID, std::string> entityToUuidMap;
		/*for (auto& root : roots)
		{
			entityToUuidMap[root] = "";
		}*/

		for (auto& entity : entities)
		{
			auto& objectInfo = scene.getComponent<ObjectInfoComponent>(entity);
			entityToUuidMap[entity] = objectInfo.uuid;
		}
		SerializationContext context{
			.uuidMap = entityToUuidMap
		};

		for (auto& entity : entities)
		{
			json entityJson;
			serializeComponent(ObjectInfoComponent);
			serializeComponent(Transform);

			serializeComponent(ModelComponent);
			serializeComponent(ImageComponent);
			serializeComponent(TextComponent);
			serializeComponent(ColliderComponent);

			serializeComponent(CameraComponent);

			serializeComponent(PointLightComponent);
			serializeComponent(DirectionalLightComponent);

			serializeComponent(FlyAIComponent);


			selectionJson["entities"].push_back(entityJson);
		}

		return selectionJson;
	}

	std::vector<EntityID> deserializeObjects(nlohmann::json objectsJson, Scene& scene, EntityID rootParent, const GlobalDeserializationContext& gContext)
	{
		std::unordered_map<std::string, EntityID> uuidToEntityMap = { {"", rootParent} };
		std::vector<EntityID> deserializedEntities;
		for (const auto& entityJson : objectsJson["entities"])
		{
			EntityID entity = scene.createEntity((EntityID)-1);
			uuidToEntityMap[entityJson["ObjectInfoComponent"]["uuid"].get<std::string>()] = entity;
			deserializedEntities.push_back(entity);
		}

		auto& ts = scene.getTransformSystem();

		DeserializationContext context{
			.shaders = gContext.shaders,
			.models = gContext.models,
			.uuidMap = uuidToEntityMap,
			.deserializeUuid = gContext.deserializeUuid
		};

		for (auto& entityJson : objectsJson["entities"])
		{
			EntityID entity = uuidToEntityMap[entityJson["ObjectInfoComponent"]["uuid"].get<std::string>()];

			deserializeExistingComponent(ObjectInfoComponent);
			deserializeExistingComponent(Transform);

			// set relationships properly
			{
				auto& transform = scene.getComponent<Transform>(entity);

				EntityID parentId = transform.parent;
				if (parentId == (EntityID)-1)
				{
					transform.parent = rootParent;
				}
				else
				{
					ts.addChild(parentId, entity);
				}

				std::vector<EntityID> children = transform.children;
				transform.children.clear();
				for (auto child : children)
				{
					ts.addChild(entity, child);
				}
			}

			deserializeComponent(ModelComponent);
			deserializeComponent(ImageComponent);
			deserializeComponent(TextComponent);
			deserializeComponent(ColliderComponent);

			deserializeComponent(CameraComponent);

			deserializeComponent(PointLightComponent);
			deserializeComponent(DirectionalLightComponent);

			deserializeComponent(FlyAIComponent);
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

	std::vector<Shader*> loadShaderList(const std::string& filePath, std::vector<Shader*>& shaders)
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