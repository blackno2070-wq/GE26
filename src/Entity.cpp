#include "Entity.h"
#include <stdexcept>
#include <sstream>
#include <cstdlib>

int Entity::nextId = 0;

Entity::Entity(EntityType type, Vec2 startPos)
    : type(type)
    , position(startPos)
    , velocity({0, 0})
    , active(true)
    , health(1)
    , id(nextId++)
{}

char Entity::display() const {
    switch (type) {
        case EntityType::TOP_PLAYER:    return 'V';
        case EntityType::BOTTOM_PLAYER: return 'M';
        case EntityType::INSECT:        return 'X';
        case EntityType::PROJECTILE:    return '|';
        default:                        return '?';
    }
}

std::string Entity::serialize() const {
    std::ostringstream ss;
    ss << id << ':' << display() << ':' << position.x << ':' << position.y << ':' << (active ? 1 : 0);
    return ss.str();
}

void Entity::move(Vec2 dir) {
    const int GRAVITY = 1;
    switch (type) {
        case EntityType::TOP_PLAYER:
            velocity.x = dir.x;
            position.x += velocity.x;
            break;
        case EntityType::BOTTOM_PLAYER:
            velocity.x = dir.x;
            velocity.y += GRAVITY;
            position = position + velocity;
            break;
        case EntityType::INSECT:
            velocity.x = -1;
            if (rand() % 6 == 0)
                velocity.y = (rand() % 3) - 1;
            position = position + velocity;
            break;
        case EntityType::PROJECTILE:
            position.y += 1;
            break;
    }
    clampPosition();
}

void Entity::clampPosition() {
    if (position.x < 0)  position.x = 0;
    if (position.x > 39) position.x = 39;
    if (position.y < 0)  position.y = 0;
    if (position.y > 19) position.y = 19;

    if (type == EntityType::TOP_PLAYER)
        position.y = 0;

    if (type == EntityType::BOTTOM_PLAYER && position.y >= 19) {
        position.y = 19;
        velocity.y = 0;
    }

    if (type == EntityType::INSECT) {
        if (position.x < 0)  position.x = 0;
        if (position.x > 39) position.x = 39;
        if (position.y > 19) position.y = 19;
    }

    if (type == EntityType::PROJECTILE && position.y >= 19)
        active = false;
}

void Entity::onCollision(Entity* other) {
    if (!other || !other->isActive()) return;

    if (type == EntityType::PROJECTILE && other->getType() == EntityType::INSECT) {
        other->takeDamage(1);
        setActive(false);
        return;
    }

    if (type == EntityType::INSECT && other->getType() == EntityType::BOTTOM_PLAYER) {
        other->takeDamage(1);
        return;
    }
}

int Entity::calculateArea() const {
    switch (type) {
        case EntityType::TOP_PLAYER:    return 1;
        case EntityType::BOTTOM_PLAYER: return 1;
        case EntityType::INSECT:        return 1;
        case EntityType::PROJECTILE:    return 1;
        default:                        return 1;
    }
}

void Entity::deserialize(const std::string& data) {
    std::stringstream ss(data);
    std::string idStr, typeChar, xStr, yStr, activeStr;
    std::getline(ss, idStr, ':');
    std::getline(ss, typeChar, ':');
    std::getline(ss, xStr, ':');
    std::getline(ss, yStr, ':');
    std::getline(ss, activeStr, ':');
    position.x = std::stoi(xStr);
    position.y = std::stoi(yStr);
    active = (activeStr == "1");
}