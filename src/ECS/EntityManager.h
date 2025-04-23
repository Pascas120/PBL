#ifndef PBL_ENTITYMANAGER_H
#define PBL_ENTITYMANAGER_H

#include <bitset>
#include <queue>
#include <iostream>
#include <cstdint>

constexpr size_t MAX_ENTITIES = 5000;
using EntityID = std::uint16_t;
class EntityManager {
public:
    EntityManager();

    EntityID CreateEntity();
    void DestroyEntity(EntityID id);
    bool IsAlive(EntityID id) const;

private:
    std::bitset<MAX_ENTITIES> aliveEntities;
    std::queue<EntityID> availableIDs;
};

#endif //PBL_ENTITYMANAGER_H