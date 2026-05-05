#include "PhysicsEngine.h"
#include "Entity.h"
#include <cstdlib>
 
// [ GAMEPLAY TASK 1 — Eman ]
 
void PhysicsEngine::init()     {}
void PhysicsEngine::shutdown() {}
 
// Moves every active entity one step by its velocity
void PhysicsEngine::updatePositions(std::vector<std::unique_ptr<Entity>>& entities)
{
    for (auto& e : entities) {
        if (!e || !e->isActive()) continue;
        Vec2 newPos = e->getPosition() + e->getVelocity();
        e->setPosition(newPos);
    }
}
 
// Checks every pair of active entities for overlap, triggers onCollision() if they touch
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