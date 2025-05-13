#include "EntityManager.h"
#include <cassert>
#include <algorithm>

EntityManager::EntityManager() {
    for (EntityID id = 0; id < MAX_ENTITIES; ++id) {
        availableIDs.push(id);
    }
}

EntityManager::EntityManager(const EntityManager& other) 
    : aliveEntities(other.aliveEntities), 
    availableIDs(other.availableIDs), 
    entities(other.entities) {}

EntityID EntityManager::createEntity() {
    assert(!availableIDs.empty() && "Maximum number of entities reached!");

    EntityID id = availableIDs.front();
    availableIDs.pop();
    aliveEntities.set(id);
	entities.push_back(id);
    return id;
}

void EntityManager::destroyEntity(EntityID id) {
    assert(id < MAX_ENTITIES && "Entity ID is out of range!");
    assert(aliveEntities.test(id) && "Attempt to destroy a non-existent entity!");

    aliveEntities.reset(id);
    availableIDs.push(id);
	std::erase(entities, id);
}

bool EntityManager::isAlive(EntityID id) const {
    assert(id < MAX_ENTITIES && "Entity ID is out of range!");
    return aliveEntities.test(id);
}