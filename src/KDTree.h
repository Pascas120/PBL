//
// Created by Łukasz Moskwin on 21/05/2025.
//

#ifndef PBL_KDTREE_H
#define PBL_KDTREE_H
#include "ECS/BoundingVolumes.h"
#include "ECS/ComponentStorage.h"


//struct BVHObject {
//    BoundingBox box;
//    Transform transform;
//    EntityID id;
//
//    BVHObject(BoundingBox b, Transform t, int i) : box(b), transform(t), id(i) {}
//};

struct BVHNode {
    BoundingBox box;
    std::unique_ptr<BVHNode> left;
    std::unique_ptr<BVHNode> right;
    BoundingVolumeComponent* object = nullptr; // tylko jeśli liść

    bool isLeaf() const { return object != nullptr; }
};

inline std::unique_ptr<BVHNode> buildBVH(std::vector<BoundingVolumeComponent*>& objects, int depth = 0) {
    if (objects.empty()) return nullptr;

    auto node = std::make_unique<BVHNode>();

    // // Początkowe AABB to pierwszy obiekt
    // node->box = objects[0]->getBoundingVolume()->getGlobalBox(*objects[0]->transform);
    // for (size_t i = 1; i < objects.size(); ++i) {
    //     auto globalBox = objects[i]->getBoundingVolume()->getGlobalBox(*objects[i]->transform);
    //     node->box = node->box.merge(globalBox);
    // }
    if (objects.size() == 1) {
        node->object = objects[0];
        node->box = objects[0]->getBoundingVolume()->getGlobalBox(*objects[0]->transform);
        return node;
    }

    // Wybór osi (X/Y/Z)
    int axis = depth % 2;
    std::sort(objects.begin(), objects.end(), [axis](BoundingVolumeComponent* a, BoundingVolumeComponent* b) {
        auto ca = a->getBoundingVolume()->getGlobalCenter(*a->transform);
        auto cb = b->getBoundingVolume()->getGlobalCenter(*b->transform);
        return (axis == 0) ? ca.x < cb.x :
               (axis == 1) ? ca.y < cb.y : ca.z < cb.z;
    });

    size_t mid = objects.size() / 2;
    std::vector<BoundingVolumeComponent*> left(objects.begin(), objects.begin() + mid);
    std::vector<BoundingVolumeComponent*> right(objects.begin() + mid, objects.end());

    node->left = buildBVH(left, depth + 1);
    node->right = buildBVH(right, depth + 1);

    if(node->left && node->right) {

        node->box = node->left->box.merge(node->right->box);
    } else if (node->left) {
        node->box = node->left->box;
    } else if (node->right) {
        node->box = node->right->box;
    }
    return node;
}

inline void traverseBVHFrustum(const BVHNode* node, const Frustum& frustum, std::vector<EntityID>& visibleIds) {
    if (!node) return;

    if (!node->box.isOnFrustum(frustum, Transform{}))
        return;

    if (node->isLeaf()) {
        if (node->object->getBoundingVolume()->isOnFrustum(frustum, *node->object->transform)) {
            visibleIds.push_back(node->object->id);
        }
    } else {
        traverseBVHFrustum(node->left.get(), frustum, visibleIds);
        traverseBVHFrustum(node->right.get(), frustum, visibleIds);
    }
}


#endif //PBL_KDTREE_H