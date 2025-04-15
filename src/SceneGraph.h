//
// Created by Łukasz Moskwin on 15/04/2025.
//

#ifndef PBL_SCENEGRAPH_H
#define PBL_SCENEGRAPH_H


#include <unordered_map>
#include <vector>

using EntityID = std::uint16_t;

class GraphNode {
private:
    EntityID id;
    EntityID parent;
    std::vector<EntityID> children;
    bool dirty;
public:
    GraphNode(EntityID id) : id(id), parent(-1), dirty(false) {}
    GraphNode(EntityID id, EntityID parent) : id(id), parent(parent), dirty(false) {}
    GraphNode() : id(0), parent(-1), dirty(false) {}
    EntityID getID() const { return id; }
    EntityID getParent() const { return parent; }
    const std::vector<EntityID>& getChildren() const { return children; }
    void setParent(EntityID parent) {
        this->parent = parent;
    }
    void addChild(EntityID child) {
        children.push_back(child);
    }
    void removeChild(EntityID child) {
        children.erase(std::remove(children.begin(), children.end(), child), children.end());
    }
    void setDirty(bool dirty) { this->dirty = dirty; }

    bool isDirty(EntityID id) const {
        return this->dirty;
    }
};

class SceneGraph {
private:
    EntityID root;
    std::unordered_map<EntityID, GraphNode> nodes;
public:
    SceneGraph() = default;

    SceneGraph(EntityID root) : root(root) {
        nodes[root] = GraphNode(root);
    }

    EntityID GetRoot() const { return root; }

    void markDirty(EntityID id) {
        if (nodes.find(id) != nodes.end()) {
            nodes[id].setDirty(true);
        }
    }

    void addNode(EntityID id, EntityID parent) {
        nodes[id] = GraphNode(id, parent);
        if (parent != -1) {
            nodes[parent].addChild(id);
        }
    }

    void removeNode(EntityID id) {
        if (nodes.find(id) != nodes.end()) {
            EntityID parent = nodes[id].getParent();
            if (parent != -1) {
                nodes[parent].removeChild(id);
            }
            nodes.erase(id);
        }
    }

    GraphNode& getNode(EntityID id) {
        return nodes.at(id);
    }

    bool isDirty(EntityID id) {
        return nodes.at(id).isDirty(id);
    }
};

#endif //PBL_SCENEGRAPH_H
