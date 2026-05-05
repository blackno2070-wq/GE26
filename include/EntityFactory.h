// EntityFactory.h
// Task 2: Entity System - Factory Design Pattern (Improved)
#pragma once
#include "Entity.h"
#include "Vec2.h"
#include <memory>
#include <string>

class EntityFactory {
public:
    /**
     * Creates an entity based on type string
     * @param type Type of entity (Player, Enemy, Projectile, PowerUp, Obstacle, Item)
     * @param id Unique ID
     * @param position Starting position
     * @param extra Extra data (e.g., username for Player)
     * @return unique_ptr to the created entity
     */
    static std::unique_ptr<Entity> createEntity(
        const std::string& type,
        int id,
        Vec2 position,
        const std::string& extra = ""
    );
};