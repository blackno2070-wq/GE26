#pragma once

#include "Vec2.h"
#include <string>

enum class EntityType {
    TOP_PLAYER,     // Rendered as 'V'  — ceiling, X-axis only
    BOTTOM_PLAYER,  // Rendered as 'M'  — ground, can jump
    INSECT,         // Rendered as 'X'  — enemy obstacle
    PROJECTILE      // Rendered as '|'  — bullet from Top Player
};

class Entity {
public:
    // ── Construction / Destruction ─────────────────────────────────
    Entity(EntityType type, Vec2 startPos);
    Entity(EntityType type, Vec2 startPos, int explicitId);  // used by factory/network
    virtual ~Entity() = default;

    virtual char display() const;
    virtual void move(Vec2 dir);

    // Keeps position inside the 40x20 arena boundaries
    void clampPosition();

    virtual void onCollision(Entity* other);
    virtual int calculateArea() const;

    virtual std::string serialize() const;
    virtual void deserialize(const std::string& data);

    // ── Getters ───────────────────────────────────────────────────
    Vec2       getPosition()  const { return position; }
    EntityType getType()      const { return type;     }
    bool       isActive()     const { return active;   }
    int        getId()        const { return id;       }
    int        getHealth()    const { return health;   }
    Vec2       getVelocity()  const { return velocity; }

    // ── Setters ───────────────────────────────────────────────────
    void setPosition(Vec2 pos) { position = pos; }
    void setActive(bool a)     { active = a;     }
    void setHealth(int hp)     { health = hp;    }
    void takeDamage(int dmg)   { health -= dmg; if (health <= 0) active = false; }

protected:
    Vec2       position;
    Vec2       velocity;
    EntityType type;
    bool       active;

private:
    int        health;
    int        id;
    static int nextId;
};