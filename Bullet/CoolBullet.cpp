#include <allegro5/base.h>
#include <random>
#include <string>

#include "Enemy/Enemy.hpp"
#include "Engine/Group.hpp"
#include "Engine/Point.hpp"
#include "CoolBullet.hpp"
#include "Scene/PlayScene.hpp"
#include "UI/Animation/DirtyEffect.hpp"

class Turret;

CoolBullet::CoolBullet(Engine::Point position, Engine::Point forwardDirection, float rotation, Turret *parent) : Bullet("play/bullet-1.png", 600, 4, position, forwardDirection, rotation - ALLEGRO_PI / 2, parent) {
}
void CoolBullet::OnExplode(Enemy *enemy) {
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist(2, 5);
    enemy->change_speed(0.6, 6.5);

    getPlayScene()->GroundEffectGroup->AddNewObject(new DirtyEffect("play/dirty-1.png", dist(rng), enemy->Position.x, enemy->Position.y));
}
