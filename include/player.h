#pragma once
#include "Entity.h"
#include "PhysicsEngine.h"
#include <string>
#include <memory>
#include <vector>

class Projectile;

class Player : public Entity {
private:
    std::string username;
    int score = 0;
    int lives = 3;
    bool isTopPlayer = false;

public:
    Player(int id, Vec2 pos, const std::string& name);

    void onCollision(Entity* other) override;
    void move(Vec2 dir) override;

    virtual std::string serialize() const override;
    virtual void deserialize(const std::string& data) override;

    std::unique_ptr<Entity> shoot();

    // Score
    int getScore() const { return score; }
    void addScore(int pts) { score += pts; }

    // Lives
    int getLives() const { return lives; }
    void loseLife() { lives--; if (lives <= 0) setActive(false); }

    // Role
    bool isTop() const { return isTopPlayer; }
    void setAsTopPlayer(bool val) { 
    isTopPlayer = val; 
    type = val ? EntityType::TOP_PLAYER : EntityType::BOTTOM_PLAYER;
}

    // Username
    const std::string& getUsername() const { return username; }

    // Velocity control for jumping
    void setVelocity(Vec2 v) { velocity = v; }
};