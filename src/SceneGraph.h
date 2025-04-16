#ifndef PBL_SCENEGRAPH_H
#define PBL_SCENEGRAPH_H

#include <unordered_map>
#include <vector>
#include <cstdint>
#include <algorithm>

using EntityID = std::uint16_t;

class GraphNode {
private:
    EntityID id;
    EntityID parent;
    std::vector<EntityID> children;
    bool dirty;

public:
    GraphNode(EntityID id);
    GraphNode(EntityID id, EntityID parent);
    GraphNode();

    EntityID getID() const;
    EntityID getParent() const;
    const std::vector<EntityID>& getChildren() const;
    void setParent(EntityID parent);
    void addChild(EntityID child);
    void removeChild(EntityID child);
    void setDirty(bool dirty);
    bool isDirty(EntityID id) const;
};

class SceneGraph {
private:
    EntityID root;
    std::unordered_map<EntityID, GraphNode> nodes;

public:
    SceneGraph();
    SceneGraph(EntityID root);

    EntityID GetRoot() const;
    void markDirty(EntityID id);
    void addNode(EntityID id, EntityID parent);
    void removeNode(EntityID id);
    GraphNode& getNode(EntityID id);
    bool isDirty(EntityID id);
};

#endif //PBL_SCENEGRAPH_H