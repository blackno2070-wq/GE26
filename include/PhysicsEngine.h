// PhysicsEngine.h
#pragma once
#include <vector>
#include <memory>
#include "Entity.h"

class PhysicsEngine {
public:
    void init();
    void shutdown();

    // Called by EntityManager::updateAll()
    void updatePositions(std::vector<std::unique_ptr<Entity>>& entities);
    void detectAndResolveCollisions(const std::vector<std::unique_ptr<Entity>>& entities);
};