//
// Created by Łukasz Moskwin on 15/04/2025.
//

#ifndef PBL_COORDINATOR_H
#define PBL_COORDINATOR_H

#include "SystemManager.h"
#include "EntityManager.h"

class Coordinator {
public:
    void Init() {
        componentManager = std::make_unique<ComponentManager>();
        entityManager = std::make_unique<EntityManager>();
        systemManager = std::make_unique<SystemManager>();
    }

    Entity CreateEntity() {
        return entityManager->CreateEntity();
    }

    void DestroyEntity(Entity entity) {
        entityManager->DestroyEntity(entity);
        componentManager->EntityDestroyed(entity);
        systemManager->EntityDestroyed(entity);
    }

    template<typename T>
    void RegisterComponent() {
        componentManager->RegisterComponent<T>();
    }

    template<typename T>
    void AddComponent(Entity entity, T component) {
        componentManager->AddComponent<T>(entity, component);

        auto signature = entitySignatures[entity];
        signature.set(componentManager->GetComponentType<T>(), true);
        entitySignatures[entity] = signature;

        systemManager->EntitySignatureChanged(entity, signature);
    }

    template<typename T>
    T& GetComponent(Entity entity) {
        return componentManager->GetComponent<T>(entity);
    }

    template<typename T>
    ComponentType GetComponentType() {
        return componentManager->GetComponentType<T>();
    }

    template<typename T>
    std::shared_ptr<T> RegisterSystem() {
        return systemManager->RegisterSystem<T>();
    }

    template<typename T>
    void SetSystemSignature(Signature signature) {
        systemManager->SetSignature<T>(signature);
    }

private:
    std::unique_ptr<ComponentManager> componentManager;
    std::unique_ptr<EntityManager> entityManager;
    std::unique_ptr<SystemManager> systemManager;
    std::unordered_map<Entity, Signature> entitySignatures{};
};



#endif //PBL_COORDINATOR_H
