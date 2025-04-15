//
// Created by Łukasz Moskwin on 15/04/2025.
//

#ifndef PBL_SYSTEMMANAGER_H
#define PBL_SYSTEMMANAGER_H

#include <unordered_map>
#include <memory>
#include <set>
#include <bitset>
#include <cassert>
#include <typeindex>
#include <typeinfo>
#include "ECS/ComponentManager.h"

class System {
public:
    std::set<Entity> entities;
};


class SystemManager {
public:
    // Rejestruje nowy system (np. MovementSystem)
    template<typename T>
    std::shared_ptr<T> RegisterSystem() {
        const char* typeName = typeid(T).name();

        assert(systems.find(typeName) == systems.end() && "System already registered.");

        auto system = std::make_shared<T>();
        systems.insert({typeName, system});
        return system;
    }

    // Nadaje systemowi "podpis" – zestaw komponentów, które system chce śledzić
    template<typename T>
    void SetSignature(Signature signature) {
        const char* typeName = typeid(T).name();

        assert(systems.find(typeName) != systems.end() && "System used before registered.");
        signatures[typeName] = signature;
    }

    // Usuwanie encji z każdego systemu, jeśli istnieje
    void EntityDestroyed(Entity entity);

    // Gdy encji zmieni się podpis (np. dodano komponent), sprawdzamy które systemy ją śledzą
    void EntitySignatureChanged(Entity entity, Signature entitySignature);

private:
    std::unordered_map<const char*, Signature> signatures;
    std::unordered_map<const char*, std::shared_ptr<System>> systems;
};



#endif //PBL_SYSTEMMANAGER_H
