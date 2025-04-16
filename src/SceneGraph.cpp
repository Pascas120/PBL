#include "SceneGraph.h"

GraphNode::GraphNode(EntityID id) : id(id), parent(-1), dirty(false) {}

GraphNode::GraphNode(EntityID id, EntityID parent) : id(id), parent(parent), dirty(false) {}

GraphNode::GraphNode() : id(0), parent(-1), dirty(false) {}

EntityID GraphNode::getID() const {
    return id;
}

EntityID GraphNode::getParent() const {
    return parent;
}

const std::vector<EntityID>& GraphNode::getChildren() const {
    return children;
}

void GraphNode::setParent(EntityID parent) {
    this->parent = parent;
}

void GraphNode::addChild(EntityID child) {
    children.push_back(child);
}

void GraphNode::removeChild(EntityID child) {
    children.erase(std::remove(children.begin(), children.end(), child), children.end());
}

void GraphNode::setDirty(bool dirty) {
    this->dirty = dirty;
}

bool GraphNode::isDirty(EntityID id) const {
    return this->dirty;
}

SceneGraph::SceneGraph() = default;

SceneGraph::SceneGraph(EntityID root) : root(root) {
    nodes[root] = GraphNode(root);
}

EntityID SceneGraph::GetRoot() const {
    return root;
}

void SceneGraph::markDirty(EntityID id) {
    if (nodes.find(id) != nodes.end()) {
        nodes[id].setDirty(true);
    }
}

void SceneGraph::addNode(EntityID id, EntityID parent) {
    nodes[id] = GraphNode(id, parent);
    if (parent != -1) {
        nodes[parent].addChild(id);
    }
}

void SceneGraph::removeNode(EntityID id) {
    if (nodes.find(id) != nodes.end()) {
        EntityID parent = nodes[id].getParent();
        if (parent != -1) {
            nodes[parent].removeChild(id);
        }
        nodes.erase(id);
    }
}

GraphNode& SceneGraph::getNode(EntityID id) {
    return nodes.at(id);
}

bool SceneGraph::isDirty(EntityID id) {
    return nodes.at(id).isDirty(id);
}