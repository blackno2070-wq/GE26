#pragma once
#include <vector>
#include "Entity.h"
#include "Player.h"

class PhysicsEngine;

class GameWorld {
private:
    std::vector<Entity*> entities;
    std::vector<Player*> players;
    PhysicsEngine* physicsEngine;

public:
    GameWorld(PhysicsEngine* engine);
    ~GameWorld();

    void spawnEntity(Entity* entity);
    void removeEntity(Entity* entity);
    void update();
    void detectCollisions();
};