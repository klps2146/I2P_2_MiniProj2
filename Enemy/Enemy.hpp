#ifndef ENEMY_HPP
#define ENEMY_HPP
#include <list>
#include <string>
#include <vector>

#include "Engine/Point.hpp"
#include "Engine/Sprite.hpp"

class Bullet;
class PlayScene;
class Turret;

class Enemy : public Engine::Sprite {
protected:
    std::vector<Engine::Point> path;
    float speed;

    bool speed_changed = false;
    float original_speed = 0;
    float speed_timer = 0;
    float speed_duration = 0;

    float hp;
    
    int money;
    PlayScene *getPlayScene();
    virtual void OnExplode();

    //// new

public:
    float reachEndTime;
    std::list<Turret *> lockedTurrets;
    std::list<Bullet *> lockedBullets;
    Enemy(std::string img, float x, float y, float radius, float speed, float hp, int money);
    void Hit(float damage);
    void UpdatePath(const std::vector<std::vector<int>> &mapDistance);
    void Update(float deltaTime) override;
    void Draw() const override;
    //// new
    void change_speed(float dv_mul, float duration);
};
#endif   // ENEMY_HPP
