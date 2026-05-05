#pragma once
#include "Entity.h"

class Enemy : public Entity {
public:
    Enemy(int id, Vec2 pos);

    // move() is inherited from Entity — uses correct INSECT drift logic
    void onCollision(Entity* other) override;

    virtual std::string serialize() const override;
    virtual void deserialize(const std::string& data) override;
};