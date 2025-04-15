//
// Created by Łukasz Moskwin on 15/04/2025.
//

#include "SystemManager.h"

void SystemManager::EntityDestroyed(Entity entity)  {
    for (auto const& pair : systems) {
        auto const& system = pair.second;
        system->entities.erase(entity);
    }
}

void SystemManager::EntitySignatureChanged(Entity entity, Signature entitySignature) {
    for (auto const& pair : systems) {
        auto const& type = pair.first;
        auto const& system = pair.second;
        auto const& systemSignature = signatures[type];

        if ((entitySignature & systemSignature) == systemSignature) {
            // Encja spełnia wymagania systemu
            system->entities.insert(entity);
        } else {
            // Już nie spełnia – usuwamy
            system->entities.erase(entity);
        }
    }
}