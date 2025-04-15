//
// Created by Łukasz Moskwin on 14/04/2025.
//

#ifndef PBL_ENTITYMANAGER_H
#define PBL_ENTITYMANAGER_H


#pragma once

#include <queue>
#include <array>
#include <cassert>

using Entity = uint16_t;
const Entity MAX_ENTITIES = 5000;

class EntityManager {
private:
    std::queue<Entity> availableEntities{};
    uint32_t livingEntityCount = 0;

public:
    EntityManager();

    Entity CreateEntity();

    void DestroyEntity(Entity entity);
};


#endif //PBL_ENTITYMANAGER_H
