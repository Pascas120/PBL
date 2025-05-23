//
// Created by Łukasz Moskwin on 21/05/2025.
//

#ifndef PBL_KDTREE_H
#define PBL_KDTREE_H
#include "ECS/BoundingVolumes.h"
#include "ECS/ComponentStorage.h"

struct KDNode{
    AABBBV bv;
    KDNode* left;
    KDNode* right;
    std::vector<EntityID> entities;

    KDNode(const AABBBV& bv) : bv(bv), left(nullptr), right(nullptr) {}
};

class KDTree{
private:
    KDNode* root;
public:
    KDTree() : root(nullptr) {}
    ~KDTree() { deleteTree(root); }

    KDNode* getRoot() const { return root; }
    void build(const ComponentStorage<BoundingVolumeComponent>& boundingVolumes, ComponentStorage<Transform>& transforms, int depth);
    void deleteTree(KDNode* node);
    void traverseTree(KDNode* node, const Frustum& frustum, std::vector<EntityID>& renderingQueue);
};


#endif //PBL_KDTREE_H
