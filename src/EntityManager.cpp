// EntityManager.cpp
// Task 2: Entity System - Manager Pattern (Improved)
#include "EntityManager.h"
#include "PhysicsEngine.h"
#include "EntityFactory.h"
#include <sstream>
#include <iostream>

void EntityManager::addEntity(std::unique_ptr<Entity> entity) {
    if (!entity) return;
    
    std::lock_guard<std::mutex> lock(mutex);
    entities.push_back(std::move(entity));
    
    std::cout << "[EntityManager] Added entity (ID: " 
              << entities.back()->getId() << ")\n";
}

void EntityManager::removeEntity(int id) {
    std::lock_guard<std::mutex> lock(mutex);
    for (auto it = entities.begin(); it != entities.end(); ++it) {
        if ((*it)->getId() == id) {
            std::cout << "[EntityManager] Removed entity ID: " << id << "\n";
            entities.erase(it);
            return;
        }
    }
}

void EntityManager::clear() {
    std::lock_guard<std::mutex> lock(mutex);
    entities.clear();
}

void EntityManager::updateAll(PhysicsEngine& physics) {
    std::lock_guard<std::mutex> lock(mutex);

    physics.updatePositions(entities);
    physics.detectAndResolveCollisions(entities);

    // Remove inactive entities
    for (auto it = entities.begin(); it != entities.end(); ) {
        if (!(*it)->isActive()) {
            it = entities.erase(it);
        } else {
            ++it;
        }
    }
}

void EntityManager::renderAll(std::vector<std::vector<char>>& grid) const {
    std::lock_guard<std::mutex> lock(mutex);
    for (const auto& e : entities) {
        if (e->isActive()) {
            Vec2 pos = e->getPosition();
grid[pos.y][pos.x] = e->display();
        }
    }
}

Entity* EntityManager::getEntityById(int id) const {
    std::lock_guard<std::mutex> lock(mutex);
    for (const auto& e : entities) {
        if (e->getId() == id) return e.get();
    }
    return nullptr;
}

std::string EntityManager::serializeAll() const {
    std::lock_guard<std::mutex> lock(mutex);
    std::string data;
    for (const auto& e : entities) {
        if (e->isActive())
            data += e->serialize() + "\n";
    }
    return data;
}

void EntityManager::deserializeAll(const std::string& data) {
    std::lock_guard<std::mutex> lock(mutex);
    std::stringstream ss(data);
    std::string line;

    while (std::getline(ss, line)) {
        if (line.empty()) continue;
        
        // Use Factory to recreate entity
        std::stringstream lineStream(line);
        std::string type;
        std::getline(lineStream, type, ',');

        int id = 0, x = 0, y = 0;
        char comma;
        lineStream >> id >> comma >> x >> comma >> y;

        auto entity = EntityFactory::createEntity(type, id, {x, y});
        if (entity) {
            entity->deserialize(line);
            entities.push_back(std::move(entity));
        }
    }
}