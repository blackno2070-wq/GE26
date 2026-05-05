#pragma once
#include "Entity.h"

class Obstacle : public Entity {
public:
    Obstacle(int id, Vec2 pos);

    void move(Vec2 dir) override;
    void onCollision(Entity* other) override;

    virtual std::string serialize() const override;
    virtual void deserialize(const std::string& data) override;
};