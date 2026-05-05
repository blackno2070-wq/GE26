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
    virtual ~Entity() = default;

    
    virtual char display() const;

   
    virtual void move(Vec2 dir);

    //  Keeps position inside the 20x20 arena boundaries [0..19].
    void clampPosition();

    // ── [ GAMEPLAY TASK 1 — Eman's responsibility ] ───────────────
    //  Called when this entity physically overlaps another.
    virtual void onCollision(Entity* other);

    //  Returns the bounding-box area used for hit detection.
    virtual int calculateArea() const;

    // ── [ NETWORK TASK 2 — Network team's responsibility ] ────────
    //  Serialises this entity's state into "ID:Type:X:Y:Active"
    //  Used by serializeAll() in EntityManager.
   virtual std::string serialize() const;
virtual void deserialize(const std::string& data);

    // ── Shared Getters (used by every team) ───────────────────────
    Vec2       getPosition()  const { return position; }
    EntityType getType()      const { return type;     }
    bool       isActive()     const { return active;   }
    int        getId()        const { return id;       }

    // ── Setters (used by Gameplay Task 4 for data security) ───────
    void setPosition(Vec2 pos) { position = pos; }
    void setActive(bool a)     { active = a;     }

    // Health getters/setters — Gameplay Task 4 enforces private access
    int  getHealth()          const { return health; }
    Vec2 getVelocity() const { return velocity; }
    void setHealth(int hp)          { health = hp;   }
    void takeDamage(int dmg)        { health -= dmg; if (health <= 0) active = false; }

protected:
    Vec2       position;
    Vec2       velocity;   // Used by Gameplay Task 2 for jump physics
    EntityType type;
    bool       active;

private:
    // ── Private Members (enforced by Gameplay Task 4) ─────────────
    int        health;
    int        id;

    static int nextId;     // Auto-incremented unique ID
};