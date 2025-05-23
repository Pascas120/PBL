//
// Created by Łukasz Moskwin on 21/05/2025.
//

#include "KDTree.h"

void KDTree::build(const ComponentStorage<BoundingVolumeComponent> &boundingVolumes, ComponentStorage<Transform>& transforms, int depth) {
    if (boundingVolumes.getQuantity()==0 || depth <= 0) {
        return;
    }

    glm::vec3 minVec(FLT_MAX, FLT_MAX, FLT_MAX);
    glm::vec3 maxVec(-FLT_MAX, -FLT_MAX, -FLT_MAX);

    for (const auto& bv : boundingVolumes) {
        auto boundingBox = std::dynamic_pointer_cast<AABBBV>(bv.GetBoundingVolume());
        const glm::vec3& center = boundingBox->getCenter();
        glm::vec3 globalCenter{ transforms.get(bv.id).globalMatrix * glm::vec4(center, 1.f) };


        const glm::vec3& extents = boundingBox->getExtents();

        glm::vec3 bvMin = globalCenter - extents;
        glm::vec3 bvMax = globalCenter + extents;

        minVec = glm::min(minVec, bvMin);
        maxVec = glm::max(maxVec, bvMax);
    }

    root = new KDNode(AABBBV(minVec, maxVec));
    root->entities.reserve(boundingVolumes.getQuantity());
    for (const auto& bv : boundingVolumes) {
        root->entities.push_back(bv.id);
    }

    int axis = depth % 3; // 0 = x, 1 = y, 2 = z

    float midValue = (minVec[axis] + maxVec[axis]) / 2.0f;

    ComponentStorage<BoundingVolumeComponent> leftVolumes;
    ComponentStorage<BoundingVolumeComponent> rightVolumes;

    for(int i = 0; i < boundingVolumes.getQuantity(); i++) {
        auto boundingBox = std::dynamic_pointer_cast<AABBBV>(boundingVolumes.components[i].GetBoundingVolume());
        const glm::vec3& center = boundingBox->getCenter();
        glm::vec3 globalCenter{ transforms.get(boundingVolumes.components[i].id).globalMatrix * glm::vec4(center, 1.f) };
        if (globalCenter[axis] < midValue) {
            leftVolumes.add(boundingVolumes.components[i].id,boundingVolumes.components[i]);
        } else {
            rightVolumes.add(boundingVolumes.components[i].id,boundingVolumes.components[i]);
        }
    }


    root->left = new KDNode(AABBBV(minVec, maxVec));
    root->right = new KDNode(AABBBV(minVec, maxVec));

    build(leftVolumes, transforms, depth - 1);
    build(rightVolumes, transforms, depth - 1);
}

void KDTree::deleteTree(KDNode *node){
    if (node) {
        deleteTree(node->left);
        deleteTree(node->right);
        delete node;
    }
}

void KDTree::traverseTree(KDNode* node, const Frustum& frustum, std::vector<EntityID>& renderingQueue) {
    if (!node) return;

    if (!node->bv.isOnFrustum(frustum, Transform())) {
        return;
    }

    if (!node->left && !node->right) {
        renderingQueue.insert(renderingQueue.end(), node->entities.begin(), node->entities.end());
        return;
    }

    traverseTree(node->left, frustum, renderingQueue);
    traverseTree(node->right, frustum, renderingQueue);
}