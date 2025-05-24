#ifndef CoolBullet_HPP
#define CoolBullet_HPP
#include "Bullet.hpp"

class Enemy;
class Turret;
namespace Engine {
    struct Point;
}   // namespace Engine

class CoolBullet : public Bullet {
public:
    explicit CoolBullet(Engine::Point position, Engine::Point forwardDirection, float rotation, Turret *parent);
    void OnExplode(Enemy *enemy) override;
};
#endif   // CoolBullet_HPP
