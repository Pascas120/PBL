//
// Created by Łukasz Moskwin on 15/04/2025.
//

#ifndef PBL_COMPONENTSTORAGE_H
#define PBL_COMPONENTSTORAGE_H
#include <assert.h>

#include "EntityManager.h"
#define MAX_OBJECTS 5000

struct IComponentStorage {
    virtual ~IComponentStorage() = default;
};

template<typename T>
class ComponentStorage : public IComponentStorage {
private:
    uint16_t quantity = 0;

public:
    using EntityID = std::uint16_t;
    T components[MAX_OBJECTS] = {};
    std::bitset<MAX_OBJECTS> exists = {0};

    bool has(EntityID id) const { return exists.test(id); }

    T &get(EntityID id) {
        for(int i = 0; i < quantity; i++) {
            if (components[i].id == id) {
                return components[i];
            }
        }
    }

    void add(EntityID id, const T &value) {
        components[quantity] = value;
		components[quantity].id = id;
        quantity++;
        exists.set(id);
    }

    void remove(EntityID id) {
        for(int i = 0; i < quantity; i++) {
            if (components[i].id == id) {
                components[i] = components[quantity - 1];
                exists.reset(id);
                quantity--;
                return;
            }
        }
    }

    uint16_t getQuantity() const {
        return quantity;
    }
};


#endif //PBL_COMPONENTSTORAGE_H
