//
// Created by lukas on 31.03.2025.
//

#ifndef SCENE_H
#define SCENE_H
#include "ECS/Components.h"
#include "ECS/ComponentStorage.h"
#include "ECS/EntityManager.h"
#include <unordered_map>
#include <typeindex>
#include <memory>

#include "ECS/TransformSystem.h"
#include "ECS/RenderingSystem.h"
#include "ECS/CollisionSystem.h"

#include "uuid.h"

class Scene {
private:
    std::unordered_map<std::type_index, std::unique_ptr<IComponentStorage>> storages;
    EntityManager entityManager;
    TransformSystem transformSystem = TransformSystem(this);
    RenderingSystem renderingSystem = RenderingSystem(this);
    CollisionSystem collisionSystem = CollisionSystem(this);
    EventSystem eventSystem = EventSystem();



    EntityID sceneGraphRoot = 0;

    template<typename T>
    ComponentStorage<T>* getOrCreateStorage() {
        auto type = std::type_index(typeid(T));
        auto it = storages.find(type);
        if (it == storages.end()) {
            auto storage = std::make_unique<ComponentStorage<T>>();
            auto ptr = storage.get();
            storages[type] = std::move(storage);
            return ptr;
        }
        return static_cast<ComponentStorage<T>*>(it->second.get());
    }
public:

    Scene() {
        entityManager = EntityManager();
        sceneGraphRoot = entityManager.createEntity();
        auto& t = addComponent<Transform>(sceneGraphRoot, Transform{});

        auto& info = addComponent<ObjectInfoComponent>(sceneGraphRoot);
		info.uuid = uuid::generate();
    }

	Scene(const Scene& other) : entityManager(other.entityManager) 
    {
		for (const auto& [type, storage] : other.storages) {
            storages[type] = std::unique_ptr<IComponentStorage>(storage->clone());
		}
    }

    Scene& operator=(const Scene&) = delete; // Wyłączenie przypisania
    Scene(Scene&&) = default; // Przenoszenie dozwolone
    Scene& operator=(Scene&&) = default;

    EntityID getSceneRootEntity() const { return sceneGraphRoot; }

    TransformSystem& getTransformSystem() {
        return transformSystem;
    }

	RenderingSystem& getRenderingSystem() {
		return renderingSystem;
	}

	CollisionSystem& getCollisionSystem() {
		return collisionSystem;
	}

    EventSystem& getEventSystem() {
        return eventSystem;
    }

    template<typename T>
    T& addComponent(EntityID id, const T& value = T{}) {
        getOrCreateStorage<T>()->add(id, value);
        return getComponent<T>(id);
    }

    template<typename T>
    void removeComponent(EntityID id) {
        auto storage = getStorage<T>();
        if (storage) {
            storage->Remove(id);
        }
    }

    template<typename T>
    bool hasComponent(EntityID id) const {
        auto storage = getStorage<T>();
        return storage && storage->has(id);
    }

    template<typename T>
    T& getComponent(EntityID id) {
        return getOrCreateStorage<T>()->get(id);
    }

    template<typename T>
    T* getComponentArray() {
        auto storage = getStorage<T>();
        if (!storage) return nullptr;
        return storage->components;
    }

    template<typename T>
    ComponentStorage<T>* getStorage() const {
        auto it = storages.find(std::type_index(typeid(T)));
        if (it == storages.end()) return nullptr;
        return static_cast<ComponentStorage<T>*>(it->second.get());
    }



    // Tworzy nowe entity, dodaje mu Transform i do grafu jako dziecko root-a
    EntityID createEntity(EntityID parent = -1) {
        EntityID id = entityManager.createEntity();
        auto& t = addComponent<Transform>(id, Transform{});
        auto& info = addComponent<ObjectInfoComponent>(id);

        if (parent < 0 || parent > MAX_ENTITIES) parent = sceneGraphRoot;
        auto& parentTransform = getComponent<Transform>(parent);
        parentTransform.children.push_back(id);

        t.parent = parent;
        info.uuid = uuid::generate();
        return id;
    }


    void destroyEntity(EntityID id) {
        if (hasComponent<Transform>(id)) {
            auto& transform = getComponent<Transform>(id);

            for (const auto& child : transform.children) {
                destroyEntity(child);
            }
            transformSystem.removeChild(transform.parent, id);
            for (auto& component : storages) {
				if (component.second->has(id)) {
					component.second->remove(id);
				}
            }
        }
        entityManager.destroyEntity(id);
    }

	bool hasEntity(EntityID id) const {
		return entityManager.isAlive(id);
	}

	const std::vector<EntityID>& getEntities() const {
		return entityManager.getEntities();
	}

};



#endif //SCENE_H
