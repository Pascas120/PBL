//
// Created by Łukasz Moskwin on 15/04/2025.
//

#ifndef PBL_COMPONENTSTORAGE_H
#define PBL_COMPONENTSTORAGE_H

#include "Export.h"

#include <assert.h>

#include "EntityManager.h"
#define MAX_OBJECTS 15000

struct IComponentStorage {
    virtual ~IComponentStorage() = default;
};

template<typename T>
class ComponentStorage : public IComponentStorage {
private:
    uint16_t quantity = 0;

public:
    using EntityID = std::uint16_t;

    T components[MAX_OBJECTS];
    int entityToIndex[MAX_OBJECTS];

    ComponentStorage() {
        for (int i = 0; i < MAX_OBJECTS; i++) {
            entityToIndex[i] = -1;
        }
    }

    bool has(EntityID id) const {
        return entityToIndex[id] != -1;
    }

    T& get(EntityID id) {
        assert(has(id) && "ComponentStorage: trying to get a non-existing component");
        return components[entityToIndex[id]];
    }

    void add(EntityID id, const T& component) {
        assert(!has(id) && "ComponentStorage: trying to add an already existing component");

        components[quantity] = component;
        components[quantity].id = id;  // upewniamy się, że id jest ustawione poprawnie
        entityToIndex[id] = quantity;
        quantity++;
    }

    void remove(EntityID id) {
        assert(has(id) && "ComponentStorage: trying to remove a non-existing component");

        int idx = entityToIndex[id];
        int lastIdx = quantity - 1;

        components[idx] = components[lastIdx];  // przepisujemy ostatni komponent na miejsce usuwanego
        entityToIndex[components[idx].id] = idx; // aktualizujemy indeks nowego komponentu na tym miejscu

        entityToIndex[id] = -1;
        quantity--;
    }

    uint16_t getQuantity() const {
        return quantity;
    }

    T* begin() { return components; }
    T* end() { return components + quantity; }

    const T* begin() const { return components; }
    const T* end() const { return components + quantity; }
};



#endif //PBL_COMPONENTSTORAGE_H
