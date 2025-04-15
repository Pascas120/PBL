#include "EntityManager.h"
#include <cassert>

EntityManager::EntityManager() {
    for (EntityID id = 0; id < MAX_ENTITIES; ++id) {
        availableIDs.push(id);
    }
}

EntityManager::EntityID EntityManager::CreateEntity() {
    assert(!availableIDs.empty() && "Maximum number of entities reached!");

    EntityID id = availableIDs.front();
    availableIDs.pop();
    aliveEntities.set(id);
    return id;
}

void EntityManager::DestroyEntity(EntityID id) {
    assert(id < MAX_ENTITIES && "Entity ID is out of range!");
    assert(aliveEntities.test(id) && "Attempt to destroy a non-existent entity!");

    aliveEntities.reset(id);
    availableIDs.push(id);
}

bool EntityManager::IsAlive(EntityID id) const {
    assert(id < MAX_ENTITIES && "Entity ID is out of range!");
    return aliveEntities.test(id);
}