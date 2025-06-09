#include "Scene.h"
#include "Application.h"

Scene::Scene(Application* app) : app(app)
{
    entityManager = EntityManager();
    sceneGraphRoot = entityManager.createEntity();
    auto& t = addComponent<Transform>(sceneGraphRoot, Transform{});

    auto& info = addComponent<ObjectInfoComponent>(sceneGraphRoot);
    info.uuid = uuid::generate();
}

Scene::Scene(const Scene& other)
    : entityManager(other.entityManager), app(other.app)
{
    for (const auto& [type, storage] : other.storages) 
    {
        storages[type] = std::unique_ptr<IComponentStorage>(storage->clone());
    }
}


EntityID Scene::createEntity(EntityID parent) {
    EntityID id = entityManager.createEntity();
    auto& t = addComponent<Transform>(id, Transform{});
    auto& info = addComponent<ObjectInfoComponent>(id);

    if (parent < 0 || parent > MAX_ENTITIES) parent = sceneGraphRoot;
    auto& parentTransform = getComponent<Transform>(parent);
    parentTransform.children.push_back(id);

    t.parent = parent;
    info.uuid = uuid::generate();
    return id;
}

void Scene::destroyEntity(EntityID id) {
    if (hasComponent<Transform>(id)) {
        auto& transform = getComponent<Transform>(id);

        for (const auto& child : transform.children) {
            destroyEntity(child);
        }
        transformSystem.removeChild(transform.parent, id);
        for (auto& component : storages) {
            if (component.second->has(id)) {
                component.second->remove(id);
            }
        }
    }
    entityManager.destroyEntity(id);
}

bool Scene::hasEntity(EntityID id) const {
    return entityManager.isAlive(id);
}

const std::vector<EntityID>& Scene::getEntities() const {
    return entityManager.getEntities();
}

std::vector<EntityID> Scene::instantiatePrefab(const std::string& prefabName, EntityID parent)
{
	return app->instantiatePrefab(prefabName, *this, parent);
}
