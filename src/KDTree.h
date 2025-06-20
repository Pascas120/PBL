//
// Created by Łukasz Moskwin on 21/05/2025.
//

#ifndef PBL_KDTREE_H
#define PBL_KDTREE_H
#include "ECS/BoundingVolumes.h"
#include "ECS/ComponentStorage.h"
#include "ECS/Components.h"
#include <optional>

static bool isOnOrForwardPlane(const BoundingBox& aabb, const Plane& plane)
{
    const float r = aabb.extents.x * std::abs(plane.normal.x) + aabb.extents.y * std::abs(plane.normal.y) +
        aabb.extents.z * std::abs(plane.normal.z);

    return -r <= plane.getSignedDistanceToPlane(aabb.center);
}

inline bool isOnFrustum(const BoundingBox& aabb, const FrustumPlanes& camFrustum)
{
	return (isOnOrForwardPlane(aabb, camFrustum.leftFace) &&
		isOnOrForwardPlane(aabb, camFrustum.rightFace) &&
		isOnOrForwardPlane(aabb, camFrustum.topFace) &&
		isOnOrForwardPlane(aabb, camFrustum.bottomFace) &&
		isOnOrForwardPlane(aabb, camFrustum.nearFace) &&
		isOnOrForwardPlane(aabb, camFrustum.farFace));
}

inline bool isOnFrustum(const BoundingBox& aabb, const FrustumPlanes& camFrustum, const glm::mat4 globalMatrix)
{
	const glm::vec3 globalCenter = glm::vec3(globalMatrix * glm::vec4(aabb.center, 1.0f));

	const glm::vec3 right = glm::vec3(globalMatrix[0]) * aabb.extents.x;
	const glm::vec3 up = glm::vec3(globalMatrix[1]) * aabb.extents.y;
	const glm::vec3 forward = glm::vec3(globalMatrix[2]) * aabb.extents.z;

	const glm::vec3 absExtents = glm::abs(right) + glm::abs(up) + glm::abs(forward);

	const BoundingBox globalAABB(globalCenter, absExtents.x, absExtents.y, absExtents.z);

	return isOnFrustum(globalAABB, camFrustum);
}

template<typename T>
struct BVHNode {
    BoundingBox box;
    std::unique_ptr<BVHNode> left;
    std::unique_ptr<BVHNode> right;
	std::optional<T> object;

	bool isLeaf() const { return object.has_value(); }
};

template<typename T>
struct TreeBox {
	BoundingBox globalBox;
    T object;
};

template<typename T>
class BVH {
	std::unique_ptr<BVHNode<T>> root;
    std::unique_ptr<BVHNode<T>> build(std::vector<TreeBox<T>>& objects, int depth)
    {
		if (objects.empty()) return nullptr;

		auto node = std::make_unique<BVHNode<T>>();

		if (objects.size() == 1) {
			node->object = objects[0].object;
			node->box = objects[0].globalBox;
			return node;
		}


		// Wybór osi (X/Y/Z)
		int axis = depth % 3;
		std::sort(objects.begin(), objects.end(), [axis](const TreeBox<T>& a, const TreeBox<T>& b) {
            auto ca = a.globalBox.center;
            auto cb = b.globalBox.center;
			return (axis == 0) ? ca.x < cb.x :
				(axis == 1) ? ca.y < cb.y : ca.z < cb.z;
		});

		size_t mid = objects.size() / 2;
		std::vector<TreeBox<T>> left(objects.begin(), objects.begin() + mid);
		std::vector<TreeBox<T>> right(objects.begin() + mid, objects.end());

		node->left = build(left, depth + 1);
		node->right = build(right, depth + 1);

		if (node->left && node->right) {
			node->box = node->left->box.merge(node->right->box);
		}
		else if (node->left) {
			node->box = node->left->box;
		}
		else if (node->right) {
			node->box = node->right->box;
		}

		return node;
    }

	void queryRecursion(const BVHNode<T>* node, const BoundingBox& box, std::vector<T>& results) const
	{
		if (!node) return;
		if (!node->box.intersects(box)) return;

		if (node->isLeaf())
		{
			results.push_back(node->object.value());
		}
		else
		{
			queryRecursion(node->left.get(), box, results);
			queryRecursion(node->right.get(), box, results);
		}
	}

public:
	void build(std::vector<TreeBox<T>>& objects)
	{
		root = build(objects, 0);
	}
	BVHNode<T>* getRoot() const { return root.get(); }

	std::vector<T> query(const BoundingBox& box) const
	{
		std::vector<T> results;
		queryRecursion(root.get(), box, results);
		return results;
	}

};


inline void traverseBVHFrustum(const BVHNode<ModelComponent*>* node, const FrustumPlanes& frustum, std::vector<EntityID>& visibleIds) {
    if (!node) return;
;
    if (!isOnFrustum(node->box, frustum))
        return;

    if (node->isLeaf()) {
		if (isOnFrustum(node->box, frustum)) {
			visibleIds.push_back(node->object.value()->id);
		}
    } else {
        traverseBVHFrustum(node->left.get(), frustum, visibleIds);
        traverseBVHFrustum(node->right.get(), frustum, visibleIds);
    }
}

#endif //PBL_KDTREE_H