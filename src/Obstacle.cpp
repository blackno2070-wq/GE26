#include "Obstacle.h"

Obstacle::Obstacle(int id, Vec2 pos)
    : Entity(EntityType::INSECT, pos) {
    setHealth(999); // Obstacles are indestructible
}

void Obstacle::move(Vec2 dir) {
    (void)dir;
    // Obstacles don't move — static objects
}

void Obstacle::onCollision(Entity* other) {
    if (!other || !other->isActive()) return;

    if (other->getType() == EntityType::BOTTOM_PLAYER ||
        other->getType() == EntityType::PROJECTILE) {
        other->takeDamage(1);
        // Push entity back
        Vec2 pos = other->getPosition();
        Vec2 vel = other->getVelocity();
        other->setPosition({pos.x - vel.x, pos.y - vel.y});
    }
}

std::string Obstacle::serialize() const {
    return Entity::serialize();
}

void Obstacle::deserialize(const std::string& data) {
    Entity::deserialize(data);
}