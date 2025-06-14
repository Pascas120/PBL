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
#include "ECS/FlyAISystem.h"

#include "uuid.h"
#include "ECS/AudioSystem.h"

class Application;

class Scene {
private:
    Application* app = nullptr;

    std::unordered_map<std::type_index, std::unique_ptr<IComponentStorage>> storages;
    EntityManager entityManager;
    TransformSystem transformSystem = TransformSystem(this);
    RenderingSystem renderingSystem = RenderingSystem(this);
    CollisionSystem collisionSystem = CollisionSystem(this);
    EventSystem eventSystem = EventSystem();
	FlyAISystem flyAISystem = FlyAISystem(this);
    AudioSystem audioSystem = AudioSystem(this);


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

    Scene(Application* app);

    Scene(const Scene& other);

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

	FlyAISystem& getFlyAISystem() {
		return flyAISystem;
	}

    AudioSystem& getAudioSystem() {
        return audioSystem;
    }

    Application* getApplication() const {
        return app;
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
            storage->remove(id);
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

    EntityID getEntityByName(const std::string& name);


    // Tworzy nowe entity, dodaje mu Transform i do grafu jako dziecko root-a
    EntityID createEntity(EntityID parent = -1);


    void destroyEntity(EntityID id);

	bool hasEntity(EntityID id) const;

	const std::vector<EntityID>& getEntities() const;

    std::vector<EntityID> instantiatePrefab(const std::string& prefabName, EntityID parent = (EntityID)-1);

};



#endif //SCENE_H
