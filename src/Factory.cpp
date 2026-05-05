#include "EntityFactory.h"
#include "Player.h"
#include "Enemy.h"
#include "Projectile.h"
#include "Obstacle.h"

std::unique_ptr<Entity> EntityFactory::createEntity(
    const std::string& type, int id, Vec2 position, const std::string& extra)
{
    std::string t = type;
    if (t == "P")  t = "Player";
    if (t == "E")  t = "Enemy";
    if (t == "PR") t = "Projectile";
    if (t == "O")  t = "Obstacle";

    if (t == "Player")     return std::make_unique<Player>(id, position, extra.empty() ? "Player1" : extra);
    if (t == "Enemy")      return std::make_unique<Enemy>(id, position);
    if (t == "Projectile") return std::make_unique<Projectile>(id, position, Vec2(0, 1));
    if (t == "Obstacle")   return std::make_unique<Obstacle>(id, position);

    return nullptr;
}