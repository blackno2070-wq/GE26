#include <iostream>
#include <algorithm>
#include "gameplay.h"
#include "PhysicsEngine.h"

// Part 1 - Eman
GameWorld::GameWorld(PhysicsEngine* engine) : physicsEngine(engine) {}

// Part 1 - Eman
GameWorld::~GameWorld() {
    for (auto entity : entities) {
        delete entity;
    }
    entities.clear();
    players.clear();
}

// Part 2 - Haneen
void GameWorld::spawnEntity(Entity* entity) {
    if (entity != nullptr) {
        entities.push_back(entity);
    }
}

void GameWorld::removeEntity(Entity* entity) {
    auto it = std::find(entities.begin(), entities.end(), entity);
    if (it != entities.end()) {
        entities.erase(it);
    }
}

// Part 4 - Abnob
void GameWorld::detectCollisions() {
    if (physicsEngine) {
        std::cout << "Checking collisions...\n";
    }
}

void GameWorld::update() {
    for (Entity* entity : entities) {
        if (entity && entity->isActive())
            entity->move({0, 0});
    }
    detectCollisions();
}