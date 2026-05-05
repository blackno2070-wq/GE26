#include "Enemy.h"
#include <cstdlib>

Enemy::Enemy(int id, Vec2 pos)
    : Entity(EntityType::INSECT, pos, id) {
    setHealth(1);
}

// move() is NOT overridden here.
// Entity::move() handles INSECT type: drifts left every tick,
// randomly shifts vertically — correct behavior, no need to duplicate.

void Enemy::onCollision(Entity* other) {
    if (!other || !other->isActive()) return;

    if (other->getType() == EntityType::PROJECTILE) {
        takeDamage(1);
    }
    if (other->getType() == EntityType::BOTTOM_PLAYER) {
        other->takeDamage(1);
    }
}

std::string Enemy::serialize() const {
    return Entity::serialize();
}

void Enemy::deserialize(const std::string& data) {
    Entity::deserialize(data);
}