//
// Created by Łukasz Moskwin on 21/05/2025.
//

#ifndef PBL_KDTREE_H
#define PBL_KDTREE_H
#include "ECS/BoundingVolumes.h"
#include "ECS/ComponentStorage.h"
#include "Ecs/RenderingSystem.h"

static bool isOnOrForwardPlane(const BoundingBox& aabb, const Plane& plane)
{
    const float r = aabb.extents.x * std::abs(plane.normal.x) + aabb.extents.y * std::abs(plane.normal.y) +
        aabb.extents.z * std::abs(plane.normal.z);

    return -r <= plane.getSignedDistanceToPlane(aabb.center);
}

inline bool isOnFrustum(const BoundingBox& aabb, const FrustumPlanes& camFrustum, const Transform& transform)
{
    const glm::vec3 globalCenter{ transform.globalMatrix * glm::vec4(aabb.center, 1.f) };
    const glm::vec3 right = transform.globalMatrix[0] * aabb.extents.x;
    const glm::vec3 up = transform.globalMatrix[1] * aabb.extents.y;
    const glm::vec3 forward = -transform.globalMatrix[2] * aabb.extents.z;

    const float newIi = std::abs(glm::dot(glm::vec3{ 1.f, 0.f, 0.f }, right)) +
        std::abs(glm::dot(glm::vec3{ 1.f, 0.f, 0.f }, up)) +
        std::abs(glm::dot(glm::vec3{ 1.f, 0.f, 0.f }, forward));

    const float newIj = std::abs(glm::dot(glm::vec3{ 0.f, 1.f, 0.f }, right)) +
        std::abs(glm::dot(glm::vec3{ 0.f, 1.f, 0.f }, up)) +
        std::abs(glm::dot(glm::vec3{ 0.f, 1.f, 0.f }, forward));

    const float newIk = std::abs(glm::dot(glm::vec3{ 0.f, 0.f, 1.f }, right)) +
        std::abs(glm::dot(glm::vec3{ 0.f, 0.f, 1.f }, up)) +
        std::abs(glm::dot(glm::vec3{ 0.f, 0.f, 1.f }, forward));

    const BoundingBox globalAABB(globalCenter, newIi, newIj, newIk);

    return (isOnOrForwardPlane(globalAABB, camFrustum.leftFace) &&
        isOnOrForwardPlane(globalAABB, camFrustum.rightFace) &&
        isOnOrForwardPlane(globalAABB, camFrustum.topFace) &&
        isOnOrForwardPlane(globalAABB, camFrustum.bottomFace) &&
        isOnOrForwardPlane(globalAABB, camFrustum.nearFace) &&
        isOnOrForwardPlane(globalAABB, camFrustum.farFace));
}


struct BVHNode {
    BoundingBox box;
    std::unique_ptr<BVHNode> left;
    std::unique_ptr<BVHNode> right;
    ModelComponent* object = nullptr;

    bool isLeaf() const { return object != nullptr; }
};

inline std::unique_ptr<BVHNode> buildBVH(std::vector<ModelComponent*>& objects, int depth = 0) {
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
        node->box = objects[0]->model->boundingBox.getGlobalBox(*objects[0]->transform);
        return node;
    }

    // Wybór osi (X/Y/Z)
    int axis = depth % 2;
    std::sort(objects.begin(), objects.end(), [axis](ModelComponent* a, ModelComponent* b) {
        auto ca = a->model->boundingBox.getGlobalCenter(*a->transform);
        auto cb = b->model->boundingBox.getGlobalCenter(*b->transform);
        return (axis == 0) ? ca.x < cb.x :
               (axis == 1) ? ca.y < cb.y : ca.z < cb.z;
    });

    size_t mid = objects.size() / 2;
    std::vector<ModelComponent*> left(objects.begin(), objects.begin() + mid);
    std::vector<ModelComponent*> right(objects.begin() + mid, objects.end());

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

inline void traverseBVHFrustum(const BVHNode* node, const FrustumPlanes& frustum, std::vector<EntityID>& visibleIds) {
    if (!node) return;
;
    if (!isOnFrustum(node->box, frustum, Transform{}))
        return;

    if (node->isLeaf()) {
        if (isOnFrustum(node->object->model->boundingBox, frustum, *node->object->transform)) {
            visibleIds.push_back(node->object->id);
        }
    } else {
        traverseBVHFrustum(node->left.get(), frustum, visibleIds);
        traverseBVHFrustum(node->right.get(), frustum, visibleIds);
    }
}


#endif //PBL_KDTREE_H