//
// Created by Łukasz Moskwin on 14/04/2025.
//

#ifndef PBL_COMPONENTARRAY_H
#define PBL_COMPONENTARRAY_H

#include <array>
#include <unordered_map>
#include <cassert>

using Entity = uint16_t;
const Entity MAX_ENTITIES = 5000;

class IComponentArray {
public:
    virtual ~IComponentArray() = default;
    virtual void EntityDestroyed(Entity entity) = 0;
};

template<typename T>
class ComponentArray: public IComponentArray {
public:
    void InsertData(Entity entity, T component) {
        assert(entityToIndexMap.find(entity) == entityToIndexMap.end() && "Component added to same entity more than once.");

        size_t newIndex = size;
        entityToIndexMap[entity] = newIndex;
        indexToEntityMap[newIndex] = entity;
        componentArray[newIndex] = component;
        ++size;
    }

    void RemoveData(Entity entity) {
        assert(entityToIndexMap.find(entity) != entityToIndexMap.end() && "Removing non-existent component.");

        size_t indexOfRemoved = entityToIndexMap[entity];
        size_t indexOfLast = size - 1;
        componentArray[indexOfRemoved] = componentArray[indexOfLast];

        Entity lastEntity = indexToEntityMap[indexOfLast];
        entityToIndexMap[lastEntity] = indexOfRemoved;
        indexToEntityMap[indexOfRemoved] = lastEntity;

        entityToIndexMap.erase(entity);
        indexToEntityMap.erase(indexOfLast);

        --size;
    }

    T& GetData(Entity entity) {
        assert(entityToIndexMap.find(entity) != entityToIndexMap.end() && "Retrieving non-existent component.");
        return componentArray[entityToIndexMap[entity]];
    }

    void EntityDestroyed(Entity entity) override {
        if (entityToIndexMap.find(entity) != entityToIndexMap.end()) {
            RemoveData(entity);
        }
    }

private:
    std::array<T, MAX_ENTITIES> componentArray{};
    std::unordered_map<Entity, size_t> entityToIndexMap{};
    std::unordered_map<size_t, Entity> indexToEntityMap{};
    size_t size = 0;
};

#endif //PBL_COMPONENTARRAY_H
