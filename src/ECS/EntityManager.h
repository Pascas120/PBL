#ifndef PBL_ENTITYMANAGER_H
#define PBL_ENTITYMANAGER_H

#include <bitset>
#include <queue>
#include <iostream>
#include <cstdint>
#include <vector>

constexpr size_t MAX_ENTITIES = 15000;
using EntityID = std::uint16_t;
class EntityManager {
public:
    EntityManager();

    EntityID createEntity();
    void destroyEntity(EntityID id);
    bool isAlive(EntityID id) const;
	const std::vector<EntityID>& getEntities() const { return entities; }

private:
    std::bitset<MAX_ENTITIES> aliveEntities;
    std::queue<EntityID> availableIDs;
	std::vector<EntityID> entities;
};

#endif //PBL_ENTITYMANAGER_H