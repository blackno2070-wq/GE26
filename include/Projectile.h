// Projectile.h
#pragma once
#include "Entity.h"

class Projectile : public Entity {
private:
    int damage = 25;

public:
    Projectile(int id, Vec2 pos, Vec2 dir, int dmg = 25);

    void onCollision(Entity* other) override;
    void move(Vec2 dir) override;

    virtual std::string serialize() const override;
    virtual void deserialize(const std::string& data) override;

    int getDamage() const { return damage; }
};