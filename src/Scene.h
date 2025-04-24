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

class Scene {
private:
    std::unordered_map<std::type_index, std::unique_ptr<IComponentStorage>> storages;
    EntityManager entityManager;
    EntityID sceneGraphRoot = 0;
    EntityID rootEntity = 0;

    template<typename T>
    ComponentStorage<T>* GetOrCreateStorage() {
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
    using EntityID = uint16_t;

    Scene() {
        entityManager = EntityManager();
        rootEntity = entityManager.CreateEntity();
        AddComponent<Transform>(rootEntity, Transform{});
        sceneGraphRoot = rootEntity;
    }

    EntityID GetSceneRootEntity() const { return sceneGraphRoot; }

    // Tworzy nowe entity, dodaje mu Transform i do grafu jako dziecko root-a
    EntityID CreateEntity(EntityID parent = -1) {
        EntityID id = entityManager.CreateEntity();
        AddComponent<Transform>(id, Transform{id});
        if (parent > 5000) parent = sceneGraphRoot;
        auto& transform = GetComponent<Transform>(parent);
        transform.children.push_back(id);
        GetComponent<Transform>(id).parent = parent;
        return id;
    }

    template<typename T>
    void AddComponent(EntityID id, const T& value) {
        GetOrCreateStorage<T>()->add(id, value);
    }

    template<typename T>
    void RemoveComponent(EntityID id) {
        auto storage = GetStorage<T>();
        if (storage) {
            storage->Remove(id);
        }
    }

    template<typename T>
    bool HasComponent(EntityID id) const {
        auto storage = GetStorage<T>();
        return storage && storage->Has(id);
    }

    template<typename T>
    T& GetComponent(EntityID id) {
        return GetOrCreateStorage<T>()->get(id);
    }

    template<typename T>
    T* GetComponentArray() {
        auto storage = GetStorage<T>();
        if (!storage) return nullptr;
        return storage->components;
    }

    template<typename T>
    ComponentStorage<T>* GetStorage() const {
        auto it = storages.find(std::type_index(typeid(T)));
        if (it == storages.end()) return nullptr;
        return static_cast<ComponentStorage<T>*>(it->second.get());
    }

};



#endif //SCENE_H
