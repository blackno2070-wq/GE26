// Projectile.cpp
#include "Projectile.h"
#include <sstream>

Projectile::Projectile(int id, Vec2 pos, Vec2 dir, int dmg)
    : Entity(EntityType::PROJECTILE, pos), damage(dmg) {
    velocity = dir;
}

void Projectile::move(Vec2 dir) {
    (void)dir;
    position.y += 3;  
    clampPosition();
}

void Projectile::onCollision(Entity* other) {
    if (!other || !other->isActive()) return;

    if (other->getType() == EntityType::INSECT) {
        other->setHealth(0);
        other->setActive(false);
        setActive(false);
    }
}

std::string Projectile::serialize() const {
    std::ostringstream ss;
    ss << Entity::serialize() << ":" << damage;
    return ss.str();
}

void Projectile::deserialize(const std::string& data) {
    Entity::deserialize(data);
}