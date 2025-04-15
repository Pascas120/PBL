//
// Created by Łukasz Moskwin on 15/04/2025.
//

#ifndef PBL_COMPONENTSTORAGE_H
#define PBL_COMPONENTSTORAGE_H
#include "EntityManager.h"
#define MAX_OBJECTS 5000

struct IComponentStorage {
    virtual ~IComponentStorage() = default;
};

template<typename T>
class ComponentStorage: public IComponentStorage {
public:
    using EntityID = std::uint16_t;
    T components[MAX_OBJECTS] = {};
    std::bitset<MAX_OBJECTS> exists;

    bool Has(EntityID id) const { return exists.test(id); }
    T& Get(EntityID id) { return components[id]; }
    void Add(EntityID id, const T& value) {
        components[id] = value;
        exists.set(id);
    }
    void Remove(EntityID id) {
        exists.reset(id);
    }
};



#endif //PBL_COMPONENTSTORAGE_H
