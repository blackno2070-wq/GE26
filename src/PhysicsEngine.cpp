#include "PhysicsEngine.h"
#include "Entity.h"
#include <cstdlib>

void PhysicsEngine::init()     {}
void PhysicsEngine::shutdown() {}

// Calls each entity's own move() — which handles position, velocity, and clamping.
// We do NOT manually add velocity here to avoid double-movement.
void PhysicsEngine::updatePositions(std::vector<std::unique_ptr<Entity>>& entities)
{
    for (auto& e : entities) {
        if (!e || !e->isActive()) continue;
        e->move({0, 0});
    }
}

// Checks every active pair for overlap (Manhattan distance <= 1), fires onCollision()
void PhysicsEngine::detectAndResolveCollisions(const std::vector<std::unique_ptr<Entity>>& entities)
{
    int n = static_cast<int>(entities.size());

    for (int i = 0; i < n; ++i) {
        Entity* a = entities[i].get();
        if (!a || !a->isActive()) continue;

        for (int j = i + 1; j < n; ++j) {
            Entity* b = entities[j].get();
            if (!b || !b->isActive()) continue;

            Vec2 posA = a->getPosition();
            Vec2 posB = b->getPosition();
            int dist  = abs(posA.x - posB.x) + abs(posA.y - posB.y);

            if (dist <= 1) {
                a->onCollision(b);
                if (b->isActive())
                    b->onCollision(a);
            }
        }
    }
}