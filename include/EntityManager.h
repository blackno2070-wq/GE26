// EntityManager.h
// Task 2: Entity System - Manager Pattern (Improved)
#pragma once
#include "Entity.h"
#include <vector>
#include <memory>
#include <mutex>

class PhysicsEngine;

class EntityManager {
private:
    std::vector<std::unique_ptr<Entity>> entities;
    mutable std::mutex mutex;

public:
    EntityManager() = default;

    // Core Management
    void addEntity(std::unique_ptr<Entity> entity);
    void removeEntity(int id);
    void clear();

    // Update & Render
    void updateAll(PhysicsEngine& physics);
    void renderAll(std::vector<std::vector<char>>& grid) const;

    // Query
    Entity* getEntityById(int id) const;
    size_t getCount() const { return entities.size(); }

    // Serialization for Networking
    std::string serializeAll() const;
    void deserializeAll(const std::string& data);

    // Utility
    const std::vector<std::unique_ptr<Entity>>& getEntities() const { return entities; }
};