//
// Created by Łukasz Moskwin on 14/04/2025.
//

#include "EntityManager.h"
EntityManager::EntityManager()  {
    for (Entity entity = 0; entity < MAX_ENTITIES; ++entity) {
        availableEntities.push(entity);
    }
}

Entity EntityManager::CreateEntity() {
    assert(livingEntityCount < MAX_ENTITIES && "Too many entities in existence.");

    Entity id = availableEntities.front();
    availableEntities.pop();
    ++livingEntityCount;

    return id;
}

void EntityManager::DestroyEntity(Entity entity) {
    assert(entity < MAX_ENTITIES && "Entity out of range.");
    assert(livingEntityCount > 0 && "No entities to destroy.");

    availableEntities.push(entity);
    --livingEntityCount;
}