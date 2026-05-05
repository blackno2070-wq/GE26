#include "Player.h"
#include "Projectile.h"
#include <sstream>

Player::Player(int id, Vec2 pos, const std::string& name)
    : Entity(EntityType::BOTTOM_PLAYER, pos, id), username(name) {
    setHealth(3);
    lives = 3;
}

void Player::onCollision(Entity* other) {
    if (!other || !other->isActive()) return;

    if (other->getType() == EntityType::INSECT) {
        loseLife();
    }
}

void Player::move(Vec2 dir) {
    if (isTopPlayer) {
        // Top player slides left/right along ceiling only
        velocity.x = dir.x;
        velocity.y = 0;
        position.x += velocity.x;
        clampPosition();
    } else {
        // Bottom player: horizontal movement + gravity/jump physics
        const int GRAVITY = 1;
        velocity.x = dir.x;
        if (position.y < 19)   // only accumulate gravity when airborne
            velocity.y += GRAVITY;
        else
            velocity.y = 0;    // reset when standing on the floor
        position = position + velocity;
        clampPosition();
    }
}

std::unique_ptr<Entity> Player::shoot() {
    // Top player shoots downward from ceiling
    Vec2 bulletDir = {0, 1};
    return std::make_unique<Projectile>(getId() * 100 + 1, position, bulletDir, 1);
}

std::string Player::serialize() const {
    std::ostringstream ss;
    ss << Entity::serialize() << ":" << username << ":" << score << ":" << lives;
    return ss.str();
}

void Player::deserialize(const std::string& data) {
    Entity::deserialize(data);
}