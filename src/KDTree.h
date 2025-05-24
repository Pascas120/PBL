//
// Created by Łukasz Moskwin on 21/05/2025.
//

#ifndef PBL_KDTREE_H
#define PBL_KDTREE_H
#include "ECS/BoundingVolumes.h"
#include "ECS/ComponentStorage.h"

struct BVHObject {
    BoundingBox box;
    Transform transform;
    EntityID id;

    BVHObject(BoundingBox b, Transform t, int i) : box(b), transform(t), id(i) {}
};

struct BVHNode {
    BoundingBox box;
    std::unique_ptr<BVHNode> left;
    std::unique_ptr<BVHNode> right;
    BVHObject* object = nullptr; // tylko jeśli liść

    bool isLeaf() const { return object != nullptr; }
};

inline std::unique_ptr<BVHNode> buildBVH(std::vector<BVHObject*>& objects, int depth = 0) {
    if (objects.empty()) return nullptr;

    auto node = std::make_unique<BVHNode>();

    // Początkowe AABB to pierwszy obiekt
    node->box = objects[0]->box;
    for (size_t i = 1; i < objects.size(); ++i)
        node->box = node->box.merge(objects[i]->box);

    if (objects.size() == 1) {
        node->object = objects[0];
        return node;
    }

    // Wybór osi (X/Y/Z)
    int axis = depth % 3;
    std::sort(objects.begin(), objects.end(), [axis](BVHObject* a, BVHObject* b) {
        auto ca = a->box.getCenter();
        auto cb = b->box.getCenter();
        return (axis == 0) ? ca.x < cb.x :
               (axis == 1) ? ca.y < cb.y : ca.z < cb.z;
    });

    size_t mid = objects.size() / 2;
    std::vector<BVHObject*> left(objects.begin(), objects.begin() + mid);
    std::vector<BVHObject*> right(objects.begin() + mid, objects.end());

    node->left = buildBVH(left, depth + 1);
    node->right = buildBVH(right, depth + 1);

    return node;
}

inline void traverseBVHFrustum(const BVHNode* node, const Frustum& frustum, std::vector<int>& visibleIds) {
    if (!node) return;

    if (!node->box.isOnFrustum(frustum, Transform{}))
        return;

    if (node->isLeaf()) {
        if (node->object->box.isOnFrustum(frustum, node->object->transform)) {
            visibleIds.push_back(node->object->id);
        }
    } else {
        traverseBVHFrustum(node->left.get(), frustum, visibleIds);
        traverseBVHFrustum(node->right.get(), frustum, visibleIds);
    }
}


#endif //PBL_KDTREE_H