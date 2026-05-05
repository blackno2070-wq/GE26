#pragma once
#include "Entity.h"

class Enemy : public Entity {
public:
    Enemy(int id, Vec2 pos);

    void move(Vec2 dir) override;
    void onCollision(Entity* other) override;

    virtual std::string serialize() const override;
    virtual void deserialize(const std::string& data) override;
};